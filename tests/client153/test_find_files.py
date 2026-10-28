"""Enumerate actual client-style paths using Win32 wildcard semantics."""
import pathlib
import subprocess
import tempfile
import unittest
ROOT = pathlib.Path(__file__).resolve().parents[2]
class FindFilesTest(unittest.TestCase):
    def test_enumeration(self):
        code = r'''#include "Windows.h"
#include <set>
#include <string>
int main(int, char **argv) {
 WIN32_FIND_DATAA d{}; auto h=FindFirstFileA(argv[1], &d);
 if(h==INVALID_HANDLE_VALUE) return 1;
 std::set<std::string> names;
 do { names.insert(d.cFileName); if(std::string(d.cFileName)=="folder" && !(d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) return 2; } while(FindNextFileA(h,&d));
 if(!FindClose(h) || names.count("plain")!=1 || names.count("old.LOG")!=1 || names.count("folder")!=1) return 3;
 h=FindFirstFileA(argv[2], &d); if(h==INVALID_HANDLE_VALUE || std::string(d.cFileName)!="old.LOG") return 4;
 if(FindNextFileA(h,&d) || !FindClose(h)) return 5;
 return FindFirstFileA(argv[3], &d)==INVALID_HANDLE_VALUE ? 0 : 6;
}'''
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp); (root/'plain').write_text('x'); (root/'old.LOG').write_text('y'); (root/'folder').mkdir()
            src = root/'test.cpp'; exe=root/'test'; src.write_text(code)
            dirs=['src/client153/platform/common', 'src/client153/Angelica2/AngelicaCommon/Header']
            subprocess.run(['c++','-std=c++17','-fshort-wchar',*(f'-I{ROOT/d}' for d in dirs),str(src),str(ROOT/'src/client153/platform/common/findfiles.cpp'),'-o',str(exe)],check=True)
            subprocess.run([str(exe),str(root)+r'\*.*',str(root)+r'\*.log',str(root)+r'\*.missing'],check=True)
if __name__ == '__main__': unittest.main()
