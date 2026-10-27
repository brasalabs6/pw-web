"""Adapt original AUI member pointers without touching inactive source."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('aui_maps', Path(__file__).resolve().parents[2] / 'scripts/qualify-client153-aui-maps.py')
aui_maps = importlib.util.module_from_spec(spec)
spec.loader.exec_module(aui_maps)


class AuiMaps(unittest.TestCase):
    def test_qualifies_each_map_class_and_preserves_comments(self):
        source = '''AUI_BEGIN_COMMAND_MAP(First, Base)
AUI_ON_COMMAND("a", OnA)
// AUI_ON_COMMAND("ignored", OnIgnored)
AUI_END_COMMAND_MAP()
/*
AUI_ON_EVENT("ignored", 1, OnIgnored)
*/
AUI_BEGIN_EVENT_MAP(Second, Base)
AUI_ON_EVENT("b", 2, OnB)
AUI_END_EVENT_MAP()
'''
        result, count = aui_maps.transform(source)
        self.assertEqual(count, 2)
        self.assertIn('AUI_ON_COMMAND("a", First::OnA)', result)
        self.assertIn('AUI_ON_EVENT("b", 2, Second::OnB)', result)
        self.assertIn('// AUI_ON_COMMAND("ignored", OnIgnored)', result)
        self.assertIn('AUI_ON_EVENT("ignored", 1, OnIgnored)', result)
        self.assertEqual(aui_maps.transform(result), (result, 0))

    def test_entry_without_map_fails(self):
        with self.assertRaises(ValueError):
            aui_maps.transform('AUI_ON_COMMAND("a", OnA)\n')


if __name__ == '__main__':
    unittest.main()
