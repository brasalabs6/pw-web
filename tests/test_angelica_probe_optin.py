"""Safety and runbook test for the local Angelica opt-in browser proof."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]

class AngelicaProbeOptInTests(unittest.TestCase):
    def test_local_probe_separated_from_regular_ci(self):
        script=(ROOT/"scripts/probe-angelica-browser.sh").read_text()
        workflow=(ROOT/".github/workflows/ci.yml").read_text()
        self.assertIn("PORT=8481",script)
        self.assertIn("artifacts/angelica-probe",script)
        self.assertIn("browser-proof.cjs",script)
        self.assertNotIn("scripts/probe-angelica.sh",workflow)
        self.assertNotIn("scripts/probe-angelica-browser.sh",workflow)

    def test_browser_checks_positive_and_negative_evidence(self):
        code=(ROOT/"tests/probes/browser-proof.cjs").read_text()
        for fragment in ('await run("positive",false)','await run("negative",true)',
                         '4096','256','1922','route.fulfill','browser-proof.json'):
            self.assertIn(fragment,code)

if __name__=="__main__":unittest.main()
