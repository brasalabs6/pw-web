#!/usr/bin/env python3
"""Reject incomplete/malformed native Grok structuredOutput before publishing any patch."""
import argparse, json
from pathlib import Path
from schema_validation import read_envelope, SchemaViolation

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--input",required=True)
    parser.add_argument("--role",required=True,choices=["coder","deepseek","glm-flash"])
    args=parser.parse_args()
    result=read_envelope(Path(args.input),args.role)
    if args.role=="coder":
        if not result["summary"].strip():
            raise SchemaViolation("Coder did not explain its work")
    else:
        if result["decision"] not in ("approved","changes_requested"):
            raise SchemaViolation("Review is pending, not final")
    print(json.dumps({"schema_valid":True,"role":args.role,"status":result["status"]}))

if __name__=="__main__":
    main()
