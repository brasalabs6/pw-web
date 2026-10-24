"""Prove all local Cloudflare models and Grok role declarations are versioned without secrets."""
import pathlib
import tomllib
import unittest

ROOT=pathlib.Path(__file__).resolve().parents[1]
MODELS={"cf-glm-5.2","cf-glm-5.3","cf-glm-5.3-flash","cf-deepseek-v4-flash",
        "cf-glm-5.3-cyber","cf-glm-5.2-cyber","cf-glm-5.3-flash-cyber","cf-qwen3.8-27b"}

class GrokModelCatalogTests(unittest.TestCase):
    def test_eight_models_proxy_and_default(self):
        raw=(ROOT/".grok/models.ci.toml").read_text()
        data=tomllib.loads(raw)
        self.assertEqual(set(data["model"]),MODELS)
        self.assertEqual(data["models"]["default"],"cf-glm-5.3")
        self.assertEqual(data["models"]["session_summary"],"cf-glm-5.3-flash")
        self.assertEqual(data["models"]["image_description"],"cf-glm-5.3-flash")
        for key, val in data["model"].items():
            self.assertEqual(val["api_backend"],"chat_completions")
            self.assertEqual(val["env_key"],"CF_PROXY_API_KEY")
            self.assertEqual(val["max_completion_tokens"],16384)
            self.assertTrue(val["model"])
            self.assertEqual(val["base_url"],"https://cf-ai-rate-proxy.guibelongtovi-a22.workers.dev/v1")
            self.assertNotIn("api_key",val)
        self.assertNotIn("cfp_key_aqui",raw)

    def test_all_three_agent_profiles(self):
        for name in ("coder","reviewer-deepseek","reviewer-glm-flash"):
            path=ROOT/".grok/agents"/(name+".md")
            source=path.read_text()
            self.assertTrue(source.startswith("---\n"))
            self.assertIn("name: pw-",source)
            self.assertIn("promptMode: extend",source)
            self.assertIn("agentsMd: true",source)

if __name__=="__main__": unittest.main()
