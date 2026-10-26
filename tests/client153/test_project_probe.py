"""Host path discovery must preserve the original Windows project semantics."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('project_probe', Path(__file__).resolve().parents[2] / 'scripts/probe-client153-project.py')
probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(probe)


class ProjectPaths(unittest.TestCase):
    def test_first_compiler_error_is_grouped_without_local_paths(self):
        output = "/tmp/example/one.cpp:4:9: fatal error: 'Windows.h' file not found\nmore\n"
        self.assertEqual(probe.first_error(output), "fatal error: 'Windows.h' file not found")
        self.assertEqual(probe.first_error('no diagnostic'), 'no compiler error diagnostic')

    def test_port_includes_platform_header_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            angelica = root / 'Angelica2'
            for name in ('AngelicaCommon/Header', 'AngelicaFile/Header', 'Angelica3D/Header'):
                (angelica / name).mkdir(parents=True)
            (root / 'platform' / 'common').mkdir(parents=True)
            self.assertIn(root / 'platform' / 'common', probe.port_include_dirs(angelica))

    def test_object_command_produces_real_object_without_filename_collisions(self):
        command = ['em++', '-fsyntax-only', 'source.cpp', '-I', 'include']
        first = probe.object_command(command, Path('/tmp/objects'), 'one/source.cpp')
        second = probe.object_command(command, Path('/tmp/objects'), 'two/source.cpp')
        self.assertNotIn('-fsyntax-only', first)
        self.assertIn('-c', first)
        self.assertNotEqual(first[-1], second[-1])
        self.assertEqual(command[1], '-fsyntax-only')

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
