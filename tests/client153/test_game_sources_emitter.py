"""The game-source manifest emitter must declare every entry and omit
nothing silently (U02): a mandatory TU absent from both trees is a hard
error with the complete inventory, the link-required extras are verified
the same way, and the browser exclusion allowlist must stay exact (a
stale entry narrows the manifest silently). The emitted fragment must
also stay free of workstation-absolute paths (the U01 CI regression)."""
import contextlib
import importlib.util
import io
import os
from pathlib import Path
import sys
import tempfile
import unittest
import unittest.mock

REPO = Path(__file__).resolve().parents[2]
EMITTER_PATH = REPO / 'scripts/emit-client153-game-sources.py'
REFERENCE = Path(os.environ.get('PW153_REFERENCE_ROOT',
                                REPO / 'references/client-153'))

spec = importlib.util.spec_from_file_location('game_sources_emitter', EMITTER_PATH)
emitter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(emitter)

VCPROJ = """<?xml version="1.0" encoding="gb2312"?>
<VisualStudioProject ProjectType="VisualC++" Version="9.00" Name="synthetic">
  <Configurations>
    <Configuration Name="Release|Win32">
      <Tool Name="VCCLCompilerTool"
            PreprocessorDefinitions="WIN32;NDEBUG"
            AdditionalIncludeDirectories="Element/Output/include"/>
    </Configuration>
    <Configuration Name="Debug|Win32">
      <Tool Name="VCCLCompilerTool"/>
    </Configuration>
  </Configurations>
  <Files>
    <File RelativePath="client.cpp"/>
    <File RelativePath="excluded_by_project.cpp">
      <FileConfiguration Name="Release|Win32" ExcludedFromBuild="true"/>
    </File>
    <File RelativePath="defence_native.cpp"/>
  </Files>
</VisualStudioProject>
"""


class SyntheticTree(unittest.TestCase):
    """Builds the minimal tree the emitter consumes and drives main()."""

    def setUp(self):
        self._directory = tempfile.TemporaryDirectory()
        self.addCleanup(self._directory.cleanup)
        # The adapted root must sit at <tmp>/src/client153 like the real
        # checkout: the emitter derives the repo root as adapted.parents[1]
        # and maps every path relative to it.
        self.root = Path(self._directory.name) / 'reference'
        self.adapted = Path(self._directory.name) / 'src/client153'
        self.output = Path(self._directory.name) / 'game-sources.cmake'
        (self.root / 'Element/CElementClient').mkdir(parents=True)
        (self.root / 'Common/AUInterface2').mkdir(parents=True)
        (self.root / 'Element/CElementClient/ElementClient.vcproj').write_text(
            VCPROJ, encoding='gb18030')
        (self.root / 'Common/AUInterface2/AUInterface_2.2.vcproj').write_text(
            VCPROJ, encoding='gb18030')
        # The vcproj-listed sources exist only in the adapted tree (the
        # emitter must pick the adapted copy and stay silent about the
        # reference, which does not carry them).
        for relative in ('Element/CElementClient/client.cpp',
                         'Element/CElementClient/defence_native.cpp',
                         'Common/AUInterface2/client.cpp',
                         'Common/AUInterface2/defence_native.cpp'):
            path = self.adapted / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('// adapted')
        # Every link-required extra must exist for the success path.
        for extra in emitter.EXTRA_SOURCES:
            path = self.adapted / extra
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('// extra')
        # The exclusion allowlist matches the synthetic projects exactly.
        self._real_exclusions = emitter.BROWSER_EXCLUDED_SOURCES
        emitter.BROWSER_EXCLUDED_SOURCES = (
            'Element/CElementClient/defence_native.cpp',
            'Common/AUInterface2/defence_native.cpp',
        )
        self.addCleanup(self._restore_exclusions)

    def _restore_exclusions(self):
        emitter.BROWSER_EXCLUDED_SOURCES = self._real_exclusions

    def run_emitter(self):
        argv = ['emit-client153-game-sources.py',
                '--source', str(self.root),
                '--adapted-root', str(self.adapted),
                '--output', str(self.output)]
        stdout, stderr = io.StringIO(), io.StringIO()
        code = None
        with unittest.mock.patch.object(sys, 'argv', argv):
            try:
                with contextlib.redirect_stdout(stdout):
                    with contextlib.redirect_stderr(stderr):
                        code = emitter.main()
            except SystemExit as exit_error:
                return exit_error.code, stdout.getvalue(), stderr.getvalue()
        return code, stdout.getvalue(), stderr.getvalue()

    def test_success_emits_portable_fragment(self):
        code, stdout, stderr = self.run_emitter()
        self.assertEqual(code, 0, stderr)
        fragment = self.output.read_text(encoding='utf-8')
        # No workstation-absolute paths (the U01 CI regression).
        self.assertNotIn('/home/', fragment)
        self.assertNotIn('/__w/', fragment)
        self.assertNotIn(str(self._directory.name), fragment)
        # The adapted copies win and the declared exclusion is applied.
        self.assertIn(
            '${PW153_REPO_ROOT}/src/client153/Element/CElementClient/client.cpp',
            fragment)
        self.assertNotIn('excluded_by_project', fragment)
        self.assertNotIn('defence_native', fragment)
        self.assertIn('exclusions: 2', stdout)  # one unique excluded path per project

    def test_missing_vcproj_source_is_a_hard_error(self):
        (self.adapted / 'Element/CElementClient/client.cpp').unlink()
        code, stdout, stderr = self.run_emitter()
        self.assertNotEqual(code, 0)
        self.assertIn('vcproj source absent from both trees: '
                      'Element/CElementClient/client.cpp', stderr)
        self.assertIn('mandatory source(s) missing', stderr)
        self.assertFalse(self.output.exists())

    def test_missing_extra_source_is_a_hard_error(self):
        (self.adapted / 'Element/CFWEditorLib/Mesh.cpp').unlink()
        code, stdout, stderr = self.run_emitter()
        self.assertNotEqual(code, 0)
        self.assertIn('link-required extra source absent from both trees: '
                      'Element/CFWEditorLib/Mesh.cpp', stderr)
        self.assertFalse(self.output.exists())

    def test_stale_exclusion_is_a_hard_error(self):
        emitter.BROWSER_EXCLUDED_SOURCES = self._real_exclusions[:1] + (
            'Element/CElementClient/never_listed.cpp',)
        code, stdout, stderr = self.run_emitter()
        self.assertNotEqual(code, 0)
        self.assertIn('stale exclusion', stderr)
        self.assertIn('Element/CElementClient/never_listed.cpp', stderr)
        self.assertFalse(self.output.exists())


    def test_reference_inside_checkout_keeps_the_reference_root(self):
        # CI parity: the reference is materialized INSIDE the checkout
        # (references/client-153). Its files must still map to the
        # overridable ${PW153_REFERENCE_ROOT}, not to
        # ${PW153_REPO_ROOT}/references/... — the byte-identity CI
        # regression (the committed fragment uses the reference root).
        repo = Path(self._directory.name) / 'repo'
        self.root = repo / 'references/client-153'
        self.adapted = repo / 'src/client153'
        (self.root / 'Element/CElementClient').mkdir(parents=True)
        (self.root / 'Common/AUInterface2').mkdir(parents=True)
        # One vcproj source exists only in the reference: the emitter must
        # emit it under the reference root.
        project = VCPROJ.replace(
            '<File RelativePath="defence_native.cpp"/>',
            '<File RelativePath="defence_native.cpp"/>\n'
            '    <File RelativePath="reference_only.cpp"/>')
        (self.root / 'Element/CElementClient/ElementClient.vcproj').write_text(
            project, encoding='gb18030')
        (self.root / 'Common/AUInterface2/AUInterface_2.2.vcproj').write_text(
            VCPROJ, encoding='gb18030')
        (self.root / 'Element/CElementClient/reference_only.cpp').write_text('// reference')
        for relative in ('Element/CElementClient/client.cpp',
                         'Element/CElementClient/defence_native.cpp',
                         'Common/AUInterface2/client.cpp',
                         'Common/AUInterface2/defence_native.cpp'):
            path = self.adapted / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('// adapted')
        for extra in emitter.EXTRA_SOURCES:
            path = self.adapted / extra
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('// extra')
        code, stdout, stderr = self.run_emitter()
        self.assertEqual(code, 0, stderr)
        fragment = self.output.read_text(encoding='utf-8')
        self.assertIn('${PW153_REFERENCE_ROOT}/Element/CElementClient/reference_only.cpp',
                      fragment)
        self.assertNotIn('${PW153_REPO_ROOT}/references/client-153', fragment)
        self.assertIn('${PW153_REPO_ROOT}/src/client153/Element/CElementClient/client.cpp',
                      fragment)


class CommittedFragment(unittest.TestCase):
    """Regenerating against the real pinned reference must reproduce the
    committed fragment byte for byte — the manifest cannot drift."""

    @unittest.skipUnless(
        (REFERENCE / 'Element/CElementClient/ElementClient.vcproj').is_file(),
        'pinned reference checkout not materialized '
        f'({REFERENCE}) — materialize references/client-153 to run this test')
    def test_committed_fragment_matches_regeneration(self):
        with tempfile.TemporaryDirectory() as directory:
            regenerated = Path(directory) / 'game-sources.cmake'
            argv = ['emit-client153-game-sources.py',
                    '--source', str(REFERENCE),
                    '--adapted-root', str(REPO / 'src/client153'),
                    '--output', str(regenerated)]
            with unittest.mock.patch.object(sys, 'argv', argv):
                self.assertEqual(emitter.main(), 0)
            committed = (REPO / 'src/client153/game-sources.cmake').read_text(
                encoding='utf-8')
            self.assertEqual(regenerated.read_text(encoding='utf-8'), committed)


if __name__ == '__main__':
    unittest.main()
