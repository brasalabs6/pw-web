import importlib.util
import pathlib
import subprocess
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
AUTO = ROOT / ".agents" / "automation"
sys.path.insert(0, str(AUTO))
from catalog import all_workorders, check_catalog, path_allowed, read
from validate_patch import check

class AgentContractTests(unittest.TestCase):
    def test_catalog_is_complete(self):
        info = check_catalog()
        self.assertGreaterEqual(info["epics"], 12)
        self.assertGreaterEqual(info["workorders"], 50)

    def test_block_unsafe_ids(self):
        for bad in ("../engine", "x/../../", "e00_unsafe", ".env", "a"):
            with self.assertRaises(ValueError):
                read(bad, "wo-01")

    def test_guarded_paths(self):
        dummy = {"write_scopes": ["web/", "engine/", "tests/"]}
        self.assertTrue(path_allowed("web/example.js", dummy))
        for path in (".github/workflows/ci.yml", ".agents/automation/catalog.py", ".grok/agents/coder.md",
                     "references/client-153/foo", "web/../../.env", "web/asset.pck",
                     "web/malware.wasm", "AGENTS.md", "scripts/verify.sh"):
            self.assertFalse(path_allowed(path, dummy))

    def test_patch_allows_only_declared_scope(self):
        dummy = {"write_scopes": ["web/"]}
        good = ("diff --git a/web/new.txt b/web/new.txt\n"
                "new file mode 100644\nindex 0000000..ce01362\n"
                "--- /dev/null\n+++ b/web/new.txt\n@@ -0,0 +1 @@\n+hello\n")
        self.assertEqual(check(good, dummy), ["web/new.txt"])
        self.assertRaises(ValueError, check, good.replace("web/new.txt", ".github/workflows/ci.yml"), dummy)
        self.assertRaises(ValueError, check, good.replace("new file mode 100644", "new file mode 120000"), dummy)

    def test_manual_workorder_is_not_dispatched(self):
        manual = [w for w in all_workorders() if not w["automation"]]
        self.assertTrue(manual)

if __name__ == "__main__":
    unittest.main()
