"""Host path discovery must preserve the original Windows project semantics."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('project_probe', Path(__file__).resolve().parents[2] / 'scripts/probe-client153-project.py')
probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(probe)


class ProjectPaths(unittest.TestCase):
    def test_case_and_parent_resolution(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'Output' / 'Include').mkdir(parents=True)
            (root / 'Source').mkdir()
            self.assertEqual(probe.resolve_windows(root / 'Source', '..\\output\\INCLUDE'), root / 'Output' / 'Include')

    def test_missing_path_remains_missing(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = probe.resolve_windows(root, 'missing\\Include')
            self.assertEqual(path, root / 'missing' / 'Include')
            self.assertFalse(path.exists())

    def test_case_collision_is_not_guessed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'Include').mkdir()
            (root / 'include').mkdir()
            with self.assertRaisesRegex(ValueError, 'ambiguous'):
                probe.resolve_windows(root, 'INCLUDE')


if __name__ == '__main__':
    unittest.main()
