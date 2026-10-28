import importlib.util
from pathlib import Path
import unittest

MODULE = Path(__file__).resolve().parents[2] / 'tools/assets/shader_inventory.py'
spec = importlib.util.spec_from_file_location('shader_inventory', MODULE)
shader_inventory = importlib.util.module_from_spec(spec)
spec.loader.exec_module(shader_inventory)


class ShaderInventoryTests(unittest.TestCase):
    def test_profile_and_instructions_ignore_comments_and_coissue_marker(self):
        data = b'; comment\r\nvs.1.1\r\nmov r0, v0 // comment\r\n+mad r1, r0, c1, c2\r\n'
        self.assertEqual(shader_inventory.analyze(data), ('vs.1.1', {'mov': 1, 'mad': 1}))

    def test_non_assembly_is_excluded(self):
        self.assertIsNone(shader_inventory.analyze(b'texture diffuse.dds\n'))


if __name__ == '__main__':
    unittest.main()
