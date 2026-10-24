import pathlib
import subprocess
import tempfile
import json
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
AUTO = ROOT / ".agents" / "automation"

class AgentIntegrationTests(unittest.TestCase):
    def call(self, script, *args):
        return subprocess.run(["python3", str(AUTO / script), *map(str, args)],
                              cwd=ROOT, capture_output=True, text=True)

    def test_dispatch_and_prompt_proof_without_model_key(self):
        with tempfile.TemporaryDirectory() as td:
            outfile = pathlib.Path(td) / "outputs.txt"
            command = self.call("validate_dispatch.py",
                               "--epic", "e01-agent-orchestration-v1",
                               "--workorder", "wo-01", "--attempt", "1",
                               "--output", outfile)
            self.assertEqual(command.returncode, 0, command.stderr)
            data = outfile.read_text()
            self.assertIn("checkout_ref=main", data)
            self.assertIn("branch=agent/e01-agent-orchestration-v1/wo-01", data)
            prompt = pathlib.Path(td) / "prompt.md"
            result = self.call("build_prompt.py",
                               "--epic", "e01-agent-orchestration-v1",
                               "--workorder", "wo-01", "--attempt", "1",
                               "--output", prompt)
            self.assertEqual(result.returncode, 0, result.stderr)
            content = prompt.read_text()
            self.assertIn("trusted assignment", content)
            self.assertIn("Read AGENTS.md", content)
            self.assertIn("write_scopes", content)

    def test_disallow_unbounded_retry_and_manual_work(self):
        with tempfile.TemporaryDirectory() as td:
            out = pathlib.Path(td) / "outputs.txt"
            for attempt in ("0", "4", "100"):
                result = self.call("validate_dispatch.py",
                                   "--epic", "e01-agent-orchestration-v1",
                                   "--workorder", "wo-01", "--attempt", attempt,
                                   "--output", out)
                self.assertNotEqual(result.returncode, 0)
            blocked = self.call("validate_dispatch.py",
                                "--epic", "e00-rights-governance-v1",
                                "--workorder", "wo-01", "--attempt", "1",
                                "--output", out)
            self.assertNotEqual(blocked.returncode, 0)

    def test_dependency_gate(self):
        with tempfile.TemporaryDirectory() as td:
            merged = pathlib.Path(td) / "merged.json"
            merged.write_text("[]")
            eligible = self.call("backlog_pick.py", "--merged", merged,
                                 "--require", "e01-agent-orchestration-v1/wo-01")
            self.assertEqual(eligible.returncode, 0, eligible.stderr)
            blocked = self.call("backlog_pick.py", "--merged", merged,
                                "--require", "e02-native-client-baseline-v1/wo-01")
            self.assertNotEqual(blocked.returncode, 0)
            self.assertIn("not eligible", blocked.stderr)
            completed = [{"title": f"agent: e01-agent-orchestration-v1 / wo-{i:02d}"} for i in range(1,6)]
            merged.write_text(json.dumps(completed))
            now = self.call("backlog_pick.py", "--merged", merged,
                            "--require", "e02-native-client-baseline-v1/wo-01")
            self.assertEqual(now.returncode, 0, now.stderr)

if __name__ == "__main__":
    unittest.main()
