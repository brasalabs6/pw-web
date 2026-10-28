import importlib.util
from pathlib import Path
import unittest


spec = importlib.util.spec_from_file_location(
    'client153_link', Path(__file__).resolve().parents[2] / 'scripts/link-client153-full.py'
)
link = importlib.util.module_from_spec(spec)
spec.loader.exec_module(link)


class LinkDiagnosticsTest(unittest.TestCase):
    def test_symbols_keep_argument_commas_and_are_deduplicated(self):
        errors = '''wasm-ld: error: /tmp/a.o: undefined symbol: foo(int, float), referenced by x
wasm-ld: error: /tmp/b.o: undefined symbol: foo(int, float), referenced by y
wasm-ld: error: /tmp/c.o: undefined symbol: foo(int, double), referenced by z
'''
        self.assertEqual(link.parse_undefined_symbols(errors), {
            'foo(int, float)': {'a.o', 'b.o'},
            'foo(int, double)': {'c.o'},
        })

    def test_unexpected_linker_failure_is_reported(self):
        self.assertIn('duplicate symbol', link.format_link_errors('wasm-ld: error: duplicate symbol: x\n'))


if __name__ == '__main__':
    unittest.main()
