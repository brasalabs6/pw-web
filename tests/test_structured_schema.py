import json, pathlib, sys, tempfile, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/".agents"/"automation"))
from schema_validation import load_schema,validate,SchemaViolation,read_envelope

class StructuredSchemaTests(unittest.TestCase):
    def test_versioned_schemas_are_valid(self):
        for role in ("coder","deepseek","glm-flash"):
            schema=load_schema(role)
            self.assertIn("status",schema["properties"])
            self.assertIn("schema_version",schema["required"])
            self.assertEqual(schema["additionalProperties"],False)

    def test_fails_closed_on_text_only_or_invalid_final(self):
        with tempfile.TemporaryDirectory() as td:
            path=pathlib.Path(td)/"result.json"
            path.write_text(json.dumps({"text":"{}","stopReason":"end_turn"}))
            with self.assertRaisesRegex(SchemaViolation,"Missing native structuredOutput"):
                read_envelope(path,"deepseek")
            good={"schema_version":1,"reviewer":"deepseek","status":"final",
                  "decision":"approved","summary":"Verified tests","findings":[],
                  "tests_assessed":[],"residual_risks":[]}
            path.write_text(json.dumps({"structuredOutput":good,"stopReason":"end_turn"}))
            self.assertEqual(read_envelope(path,"deepseek")["decision"],"approved")
            path.write_text(json.dumps({"structuredOutput":{**good,"status":"working"},"stopReason":"end_turn"}))
            with self.assertRaises(SchemaViolation):
                read_envelope(path,"deepseek")
            path.write_text(json.dumps({"structuredOutput":good,"stopReason":"max_tokens"}))
            with self.assertRaises(SchemaViolation):
                read_envelope(path,"deepseek")
            with self.assertRaises(SchemaViolation):
                validate({**good,"unexpected":1},load_schema("deepseek"))

    def test_schema_rejects_wrong_role_or_missing_finding_fields(self):
        sample={"schema_version":1,"reviewer":"glm-flash","status":"final",
                "decision":"changes_requested","summary":"Bug","findings":[],
                "tests_assessed":[],"residual_risks":[]}
        with self.assertRaises(SchemaViolation):
            validate(sample,load_schema("deepseek"))
        sample["reviewer"]="deepseek"
        sample["findings"]=[{"severity":"high","path":"web/app.js"}]
        with self.assertRaises(SchemaViolation):
            validate(sample,load_schema("deepseek"))

if __name__=="__main__": unittest.main()
