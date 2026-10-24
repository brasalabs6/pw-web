"""Bounded JSON Schema 2020-12 subset for deterministic, no-network CI validation.

Supported keywords are deliberately explicit. Unknown schema keywords (except annotations)
raise instead of silently weakening the schema. This is independent of model compliance.
"""
from __future__ import annotations
import json
from pathlib import Path

SCHEMAS = Path(__file__).resolve().parents[2] / ".grok" / "schemas"
ANNOTATIONS = {"$schema", "$id", "title", "description", "default", "examples"}
KEYWORDS = ANNOTATIONS | {"type","const","enum","required","properties","additionalProperties","items","maxItems","minItems","minLength","maxLength","minimum","maximum"}

class SchemaViolation(ValueError):
    pass

def load_schema(role: str) -> dict:
    paths = {"coder":"coder.schema.json","deepseek":"reviewer-deepseek.schema.json",
             "glm-flash":"reviewer-glm-flash.schema.json"}
    if role not in paths:
        raise SchemaViolation("Unknown agent schema role")
    obj = json.loads((SCHEMAS / paths[role]).read_text(encoding="utf-8"))
    validate_schema(obj)
    return obj

def validate_schema(spec: dict) -> None:
    if not isinstance(spec, dict):
        raise SchemaViolation("Schema must be object")
    unknown = set(spec) - KEYWORDS
    if unknown:
        raise SchemaViolation("Unsupported schema keywords: " + repr(unknown))
    for child in spec.get("properties", {}).values():
        validate_schema(child)
    if isinstance(spec.get("items"), dict):
        validate_schema(spec["items"])

def validate(value, spec: dict, where: str = "$") -> None:
    ty=spec.get("type")
    allowed=ty if isinstance(ty,list) else [ty] if ty else []
    def correct(t: str) -> bool:
        return {"object":lambda:isinstance(value,dict),"array":lambda:isinstance(value,list),
                "string":lambda:isinstance(value,str),"integer":lambda:type(value) is int,
                "number":lambda:type(value) in (int,float),
                "boolean":lambda:type(value) is bool,"null":lambda:value is None}[t]()
    if allowed and not any(correct(t) for t in allowed):
        raise SchemaViolation(f"{where}: expected {allowed}, got {type(value).__name__}")
    if "const" in spec and value != spec["const"]:
        raise SchemaViolation(f"{where}: wrong const")
    if "enum" in spec and value not in spec["enum"]:
        raise SchemaViolation(f"{where}: not in enum")
    if isinstance(value,dict):
        for key in spec.get("required",[]):
            if key not in value:
                raise SchemaViolation(f"{where}.{key}: required field absent")
        properties=spec.get("properties",{})
        if spec.get("additionalProperties") is False and set(value)-set(properties):
            raise SchemaViolation(f"{where}: unexpected fields")
        for k,v in value.items():
            if k in properties:
                validate(v,properties[k],f"{where}.{k}")
    elif isinstance(value,list):
        if len(value)>spec.get("maxItems",float("inf")) or len(value)<spec.get("minItems",0):
            raise SchemaViolation(f"{where}: invalid item count")
        if "items" in spec:
            for i,v in enumerate(value):
                validate(v,spec["items"],f"{where}[{i}]")
    elif isinstance(value,str):
        if len(value)>spec.get("maxLength",float("inf")) or len(value)<spec.get("minLength",0):
            raise SchemaViolation(f"{where}: string length out of bounds")
    elif type(value) in (int,float):
        if value<spec.get("minimum",-float("inf")) or value>spec.get("maximum",float("inf")):
            raise SchemaViolation(f"{where}: numeric bound violated")

def read_envelope(path: Path, role: str) -> dict:
    env=json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(env,dict):
        raise SchemaViolation("Grok JSON envelope is not an object")
    if env.get("structuredOutputError"):
        raise SchemaViolation("Provider rejected structured output: "+str(env["structuredOutputError"])[:250])
    if env.get("stopReason")!="end_turn":
        raise SchemaViolation("Grok did not complete normally: "+str(env.get("stopReason")))
    value=env.get("structuredOutput")
    if not isinstance(value,dict):
        raise SchemaViolation("Missing native structuredOutput. text fallback is forbidden.")
    validate(value,load_schema(role))
    if value["status"]!="final":
        raise SchemaViolation("Agent response still working; no final evidence")
    return value
