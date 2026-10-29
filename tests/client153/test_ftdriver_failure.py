"""The unavailable UI font driver must report its actual boot impact."""
import pathlib
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]


def _find_compiler():
    """The CI runners differ in which C++ front end is on PATH; any C++17
    compiler works for this probe (clang++ preferred, then c++/g++)."""
    for candidate in ('clang++', 'c++', 'g++'):
        path = shutil.which(candidate)
        if path:
            return path
    raise unittest.SkipTest('no C++ compiler on PATH')


class FTDriverFailureTest(unittest.TestCase):
    def test_error_does_not_claim_ui_font_fallback(self):
        compiler = _find_compiler()
        with tempfile.TemporaryDirectory() as directory:
            source = pathlib.Path(directory) / 'main.cpp'
            binary = pathlib.Path(directory) / 'probe'
            source.write_text('#include "FTInterface.h"\nint main() { return CreateFTManager() ? 1 : 0; }\n')
            subprocess.run([compiler, '-std=c++17', '-fshort-wchar', '-fms-extensions',
                '-I', str(ROOT / 'src/client153/platform/common'),
                '-I', str(ROOT / 'src/client153/Angelica2/AngelicaCommon/Header'),
                '-include', str(ROOT / 'src/client153/platform/common/Windows.h'),
                '-I', str(ROOT / 'src/client153/Common/AUInterface2'), str(source),
                str(ROOT / 'src/client153/thirdparty/ftdriver/ftdriver_stub.cpp'), '-o', str(binary)],
                check=True, capture_output=True)
            result = subprocess.run([str(binary)], check=True, capture_output=True, text=True)
            self.assertIn('login UI cannot initialize', result.stderr)
            self.assertNotIn('falls back', result.stderr)

if __name__ == '__main__': unittest.main()
