"""Repository-local create-pr governance and browser port contract."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]

class CreatePrGovernanceTests(unittest.TestCase):
    def test_skill_is_discoverable_from_agents_and_has_required_contract(self):
        skill=(ROOT/".agent/skills/create-pr/SKILL.md").read_text(encoding="utf-8")
        agents=(ROOT/"AGENTS.md").read_text(encoding="utf-8")
        template=(ROOT/".github/PULL_REQUEST_TEMPLATE.md").read_text(encoding="utf-8")
        self.assertTrue(skill.startswith("---\nname: create-pr\n"))
        self.assertIn(".agent/skills/create-pr/SKILL.md",agents)
        for term in ("workorder","draft","review","CI","direitos","8480","8481"):
            self.assertIn(term.lower(),skill.lower())
        for heading in ("Resumo e hipótese","Evidências reproduzíveis","Revisão e findings",
                        "Direitos, segurança e limitações","Rollback e próximos passos"):
            self.assertIn(heading,template)

    def test_all_browser_entrypoints_avoid_reserved_8080(self):
        paths=("scripts/dev.sh","scripts/browser-smoke.sh","scripts/chrome-mcp-launch.sh",
               "tests/browser_smoke.cjs",".grok/config.toml",
               ".github/workflows/agent-execute.yml",
               ".github/workflows/agent-review.yml",
               "docs/BROWSER_TESTING.md")
        for path in paths:
            contents=(ROOT/path).read_text(encoding="utf-8")
            self.assertNotIn("http://127.0.0.1:8080",contents,path)
            self.assertNotIn("http://localhost:8080",contents,path)
            self.assertNotIn("http.server 8080",contents,path)
            self.assertIn("8480",contents,path)

if __name__=="__main__":unittest.main()
