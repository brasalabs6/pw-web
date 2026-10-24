"""Tests needing neither private source code nor game assets."""
import pathlib
import subprocess
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]

class BootstrapTests(unittest.TestCase):
    def test_reference_isolation(self):
        modules = (ROOT / ".gitmodules").read_text(encoding="utf-8")
        self.assertIn("references/client-153", modules)
        self.assertIn("references/client-152-modern", modules)
        self.assertIn("references/server-rust", modules)
        for name in ("client-153", "client-152-modern", "server-rust"):
            self.assertIn(f"references/{name}", modules)

    def test_safe_gitignore(self):
        ignore = (ROOT / ".gitignore").read_text(encoding="utf-8")
        for mask in ("*.pck", "*.pkx", "*.dds", "*.ecm", ".env"):
            self.assertIn(mask, ignore)
        self.assertIn("web/wasm/*.wasm", ignore)

    def test_core_boundaries(self):
        sources = "\n".join((ROOT / "engine/core" / name).read_text(encoding="utf-8")
                            for name in ("pw_core.cpp", "pw_core.h"))
        for banned in ("<Windows.h>", "<d3d8.h>", "<d3d9.h>", "<winsock2.h>"):
            self.assertNotIn(banned, sources)

    def test_documentation(self):
        for path in (
            "README.md", "AGENTS.md", "docs/ONBOARDING.md", "docs/INITIATION.md",
            "docs/ARCHITECTURE.md", "docs/ROADMAP.md",
            "docs/RESEARCH.md", "docs/VALIDATION.md", "docs/LEGAL.md",
        ):
            with self.subTest(path=path):
                self.assertGreater((ROOT / path).stat().st_size, 100)

    def test_no_game_assets_tracked(self):
        output = subprocess.run(
            ["git", "ls-files"], cwd=ROOT, capture_output=True, text=True, check=True,
        ).stdout
        assets = (".pck", ".pkx", ".ecm", ".bon", ".ski", ".smd", ".dds")
        forbidden = [line for line in output.splitlines()
                     if line.lower().endswith(assets)]
        self.assertEqual(forbidden, [], f"Original game assets committed: {forbidden}")

if __name__ == "__main__":
    unittest.main()
