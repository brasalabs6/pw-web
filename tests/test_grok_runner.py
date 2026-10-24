"""Validate model selection, sandbox flags and secret handling without calling the proxy."""
import json
import os
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
RUNNER = ROOT / "scripts" / "run-grok-workorder.sh"

class GrokBuildRunnerTests(unittest.TestCase):
    def mock_runner(self, tmpdir):
        binary = pathlib.Path(tmpdir) / "grok"
        binary.write_text(
            '#!/usr/bin/env python3\n'
            'import json, os, sys\n'
            'with open(os.environ["TEST_GROK_ARGS"], "w") as f: json.dump(sys.argv[1:], f)\n'
            'print(json.dumps({"text": "Mock Grok Build output", "stopReason": "end_turn", "structuredOutput": {"schema_version":1,"agent":"coder","status":"final","summary":"Fixture","files_changed":[],"tests_run":[],"limitations":[]}}))\n'
        )
        binary.chmod(0o755)

    def test_forwards_profile_model_prompt_and_sandbox(self):
        with tempfile.TemporaryDirectory() as td:
            self.mock_runner(td)
            (pathlib.Path(td) / "config.toml").write_bytes((ROOT / ".grok/models.ci.toml").read_bytes())
            prompt = pathlib.Path(td) / "prompt.md"
            result = pathlib.Path(td) / "result.json"
            trace = pathlib.Path(td) / "args.json"
            prompt.write_text("Implement safe fixture with tests.")
            env = {**os.environ, "PATH": td + os.pathsep + os.environ.get("PATH", ""),
                   "CF_PROXY_API_KEY": "test-placeholder-not-real", "GROK_HOME": td,
                   "TEST_GROK_ARGS": str(trace)}
            proc = subprocess.run(["bash", str(RUNNER), str(prompt), str(result)],
                                  env=env, cwd=ROOT, capture_output=True, text=True)
            self.assertEqual(proc.returncode, 0, proc.stderr)
            args = json.loads(trace.read_text())
            for flag in ("--sandbox", "--always-approve", "--no-auto-update",
                         "--disable-web-search", "--no-memory", "--agent"):
                self.assertIn(flag, args)
            self.assertEqual(args[args.index("--sandbox")+1], "strict")
            self.assertEqual(args[args.index("--max-turns")+1], "35")
            self.assertIn("--json-schema", args)
            self.assertTrue(args[args.index("--json-schema")+1].endswith("coder.schema.json"))
            self.assertIn("--prompt-file", args)
            self.assertEqual(args[args.index("--prompt-file")+1], str(prompt))
            self.assertEqual(args[args.index("-m")+1], "cf-glm-5.3")
            self.assertIn("coder.md", args[args.index("--agent")+1])
            self.assertEqual(json.loads(result.read_text())["text"], "Mock Grok Build output")
            self.assertNotIn("test-placeholder-not-real", proc.stdout+proc.stderr)

    def test_missing_proxy_key_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            self.mock_runner(td)
            (pathlib.Path(td) / "config.toml").write_bytes((ROOT / ".grok/models.ci.toml").read_bytes())
            p = pathlib.Path(td) / "prompt.md"
            p.write_text("hello")
            env = {**os.environ, "PATH": td + os.pathsep + os.environ.get("PATH", ""),
                   "GROK_HOME": td}
            env.pop("CF_PROXY_API_KEY", None)
            proc = subprocess.run(["bash", str(RUNNER), str(p), str(pathlib.Path(td)/"result")],
                                  env=env, cwd=ROOT, capture_output=True, text=True)
            self.assertNotEqual(proc.returncode, 0)
            self.assertIn("CF_PROXY_API_KEY", proc.stderr)

if __name__ == "__main__":
    unittest.main()
