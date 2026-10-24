import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / ".agents/automation"))
from backlog_pick import completed_from_prs, completed_from_manifest, eligibles

class BacklogTests(unittest.TestCase):
    def test_parse_trusted_merged_titles(self):
        prs = [
            {"title":"agent: e01-agent-orchestration-v1 / wo-01"},
            {"title":"workorder: e00-rights-governance-v1/wo-01"},
            {"title":"unrelated: wrong"},
        ]
        got = completed_from_prs(prs)
        self.assertIn("e01-agent-orchestration-v1/wo-01", got)
        self.assertIn("e00-rights-governance-v1/wo-01", got)
        self.assertEqual(len(got),2)

    def test_deterministic_first_eligible(self):
        ready = eligibles(completed_from_manifest())
        self.assertTrue(ready)
        self.assertEqual(ready[0]["epic"],"e01-agent-orchestration-v1")
        self.assertEqual(ready[0]["id"],"wo-01")
        self.assertFalse(any(w["epic"]=="e00-rights-governance-v1" for w in ready))

    def test_completed_block_duplicates(self):
        done = {"e01-agent-orchestration-v1/wo-01"}
        ready = eligibles(done)
        self.assertEqual(ready[0]["id"],"wo-02")

if __name__ == "__main__":
    unittest.main()
