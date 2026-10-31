"""Host path discovery must preserve the original Windows project semantics."""
import importlib.util
from pathlib import Path
import tempfile
import json
import subprocess
import sys
import unittest

spec = importlib.util.spec_from_file_location('project_probe', Path(__file__).resolve().parents[2] / 'scripts/probe-client153-project.py')
probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(probe)


class ProjectPaths(unittest.TestCase):
    def test_project_include_list_supports_comma_and_semicolon(self):
        self.assertEqual(probe.split_includes('header,../media;../common'),
                         ['header', '../media', '../common'])

    def test_aui_prelude_is_not_applied_to_engine_project(self):
        self.assertFalse(probe.uses_aui_prelude('Angelica2/Angelica3D/Angelica3D.vcproj'))
        self.assertTrue(probe.uses_aui_prelude('Common/AUInterface2/AUInterface_2.2.vcproj'))
        self.assertTrue(probe.uses_aui_prelude('Element\\CElementClient\\ElementClient.vcproj'))

    def test_uses_toolchain_zlib_instead_of_windows_sdk_zlib(self):
        root = Path('/reference')
        windows_zlib = root / 'Angelica2/Output/3rdsdk/include/zlib'
        other = root / 'other/zlib'
        self.assertEqual(probe.portable_include_dirs(root, [windows_zlib, other]), [other])

    def test_prefers_adapted_source_and_falls_back_to_reference(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            reference = root / 'reference'
            adapted = root / 'adapted'
            relative = Path('Element/client.cpp')
            (reference / relative).parent.mkdir(parents=True)
            (reference / relative).write_text('original')
            self.assertEqual(probe.source_path(reference, adapted, relative), reference / relative)
            (adapted / relative).parent.mkdir(parents=True)
            (adapted / relative).write_text('adapted')
            self.assertEqual(probe.source_path(reference, adapted, relative), adapted / relative)
            self.assertEqual(probe.source_path(reference, None, relative), reference / relative)

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


    def test_extra_project_generates_objects_and_reports_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            main = root / 'main'
            extra = root / 'extra'
            main.mkdir()
            extra.mkdir()
            project = """<VisualStudioProject><Configurations><Configuration Name="Release|Win32"><Tool Name="VCCLCompilerTool" AdditionalIncludeDirectories="." PreprocessorDefinitions=""/></Configuration></Configurations><Files><File RelativePath="{source}"/></Files></VisualStudioProject>"""
            (main / 'main.vcproj').write_text(project.format(source='ElementClient.cpp'))
            (extra / 'extra.vcproj').write_text(project.format(source='library.cpp'))
            (main / 'ElementClient.cpp').write_text('int main_source() { return 0; }')
            library = extra / 'library.cpp'
            library.write_text('int library_source() { return 1; }')
            wrapper = root / 'compiler'
            wrapper.write_text('#!/usr/bin/env python3\nimport os, sys\na=[x for x in sys.argv[1:] if x not in ("-sUSE_ZLIB=1", "-fms-extensions", "-ferror-limit=5")]\nos.execvp("c++", ["c++"]+a)\n')
            wrapper.chmod(0o755)
            output = root / 'report.json'
            objects = root / 'objects'
            command = [sys.executable, str(Path(probe.__file__)), '--source', str(root),
                       '--project', 'main/main.vcproj', '--extra-project', str(extra / 'extra.vcproj'),
                       '--emxx', str(wrapper), '--output', str(output), '--all', '--object-dir', str(objects)]
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(len(list(objects.glob('*.o'))), 2)
            single_objects = root / 'single-objects'
            single = [sys.executable, str(Path(probe.__file__)), '--source', str(root),
                      '--project', 'main/main.vcproj', '--emxx', str(wrapper),
                      '--output', str(output), '--object-dir', str(single_objects)]
            result = subprocess.run(single, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(len(list(single_objects.glob('*.o'))), 1)
            (main / 'ElementClient.cpp').write_text('#error broken entry\n')
            result = subprocess.run(single, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(len(list(single_objects.glob('*.o'))), 0)
            (main / 'ElementClient.cpp').write_text('int main_source() { return 0; }')
            library.write_text('#error extra compilation failed\n')
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(len(list(objects.glob('*.o'))), 1)
            report = json.loads(output.read_text())
            self.assertNotEqual(report['extraProjects'][0]['sources'][0]['exitCode'], 0)

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
