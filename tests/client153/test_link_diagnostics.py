import importlib.util
from pathlib import Path
import unittest
import tempfile
import hashlib


spec = importlib.util.spec_from_file_location(
    'client153_link', Path(__file__).resolve().parents[2] / 'scripts/link-client153-full.py'
)
link = importlib.util.module_from_spec(spec)
spec.loader.exec_module(link)


class LinkDiagnosticsTest(unittest.TestCase):
    def test_current_aui_objects_replace_campaign_objects(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = 'Common/AUInterface2/A3DFTFont.cpp'
            old = root / (hashlib.sha256(source.encode()).hexdigest() + '.o')
            old.touch()
            other = root / 'other.o'
            other.touch()
            current = root / 'aui' / (source + '.o')
            current.parent.mkdir(parents=True)
            current.touch()
            self.assertEqual(link.overlay_aui_objects([old, other], root / 'aui'), [current, other])

    def test_fresh_entry_object_replaces_stale_campaign_object(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = 'Element/CElementClient/ElementClient.cpp'
            old = root / (hashlib.sha256(source.encode()).hexdigest() + '.o')
            old.touch()
            current = root / 'current.o'
            current.touch()
            self.assertEqual(link.replace_source_object([old], source, current), [current])

    def test_symbols_keep_argument_commas_and_are_deduplicated(self):
        errors = '''wasm-ld: error: /tmp/a.o: undefined symbol: foo(int, float), referenced by x
wasm-ld: error: /tmp/b.o: undefined symbol: foo(int, float), referenced by y
wasm-ld: error: /tmp/c.o: undefined symbol: foo(int, double), referenced by z
'''
        self.assertEqual(link.parse_undefined_symbols(errors), {
            'foo(int, float)': {'a.o', 'b.o'},
            'foo(int, double)': {'c.o'},
        })

    def test_entry_probe_keeps_explicit_root_without_forcing_unused_sections(self):
        self.assertEqual(link.retention_flags("_Z8wWinMainPvS_Pwi"),
                         ["-Wl,--export=_Z8wWinMainPvS_Pwi"])
        self.assertEqual(link.retention_flags(None), ["-Wl,--no-gc-sections"])

    def test_node_probe_flags_export_malloc_and_preserve_js_path(self):
        self.assertEqual(link.node_probe_flags("/tmp/probe.js"),
                         ["--pre-js", str(link.ROOT / 'scripts/client153-readonly-node.js'),
                          "--pre-js", "/tmp/probe.js", "-sEXPORTED_FUNCTIONS=_malloc",
                          "-sALLOW_MEMORY_GROWTH=1"])
        self.assertEqual(link.node_probe_flags(None), [])

    def test_unexpected_linker_failure_is_reported(self):
        self.assertIn('duplicate symbol', link.format_link_errors('wasm-ld: error: duplicate symbol: x\n'))


if __name__ == '__main__':
    unittest.main()
