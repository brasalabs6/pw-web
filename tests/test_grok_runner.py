"""Prove CLI flag forwarding without an xAI API call or a real API credential."""
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
            'print(json.dumps({"text": "Mock Grok Build output", "stopReason": "end_turn"}))\n'
        )
        binary.chmod(0o755)
        return binary

    def test_forwards_prompt_and_sandbox_controls(self):
        with tempfile.TemporaryDirectory() as td:
            self.mock_runner(td)
            (pathlib.Path(td) / "config.toml").write_text("[models]\\ndefault = \\"cf-glm-5.3\\"\\n")
            prompt = pathlib.Path(td) / "prompt.md"
            result = pathlib.Path(td) / "result.json"
            trace = pathlib.Path(td) / "args.json"
            prompt.write_text("Implement safe fixture with tests.")
            env = {**os.environ, "PATH": td + os.pathsep + os.environ.get("PATH", ""),
                   "CF_PROXY_API_KEY": "test-key-not-real", "GROK_HOME": td, "TEST_GROK_ARGS": str(trace)}
            proc = subprocess.run(["bash", str(RUNNER), str(prompt), str(result)],
                                  env=env, cwd=ROOT, capture_output=True, text=True)
            self.assertEqual(proc.returncode, 0, proc.stderr)
            args = json.loads(trace.read_text())
            self.assertIn("--sandbox", args)
            self.assertEqual(args[args.index("--sandbox") + 1], "strict")
            self.assertIn("--always-approve", args)
            self.assertIn("--no-auto-update", args)
            self.assertIn("--disable-web-search", args)
            self.assertIn("--no-memory", args)
            self.assertEqual(args[args.index("--max-turns")+1], "35")
            self.assertEqual(args[args.index("-p")+1], prompt.read_text())\n            self.assertEqual(args[args.index("-m")+1], "cf-glm-5.3")\n            self.assertIn("--agent-profile",args)\n            self.assertIn("coder.md",args[args.index("--agent-profile")+1])
            self.assertEqual(json.loads(result.read_text())["text"], "Mock Grok Build output")
            self.assertNotIn("test-key-not-real", proc.stdout+proc.stderr)

    def test_missing_api_key_fails_closed(self):
        with tempfile.TemporaryDirectory() as td:
            self.mock_runner(td)
            prompt = pathlib.Path(td) / "p"
            prompt.write_text("hello")
            env = {**os.environ, "PATH": td + os.pathsep + os.environ.get("PATH", ""), "GROK_HOME": td}
            env.pop("CF_PROXY_API_KEY", None)
            proc = subprocess.run(["bash", str(RUNNER), str(prompt), str(pathlib.Path(td)/"r")],
                                  env=env, capture_output=True, text=True)
            self.assertNotEqual(proc.returncode, 0)
            self.assertIn("CF_PROXY_API_KEY", proc.stderr)

if __name__ == "__main__":
    unittest.main()
