from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

SCRIPT=Path(__file__).resolve().parents[2]/'scripts/prepare-client153-scene.py'
class StagingBoundaryTests(unittest.TestCase):
    def test_output_cannot_modify_any_part_of_original_app(self):
        with tempfile.TemporaryDirectory() as root:
            app=Path(root)/'app';app.mkdir()
            result=subprocess.run([sys.executable,str(SCRIPT),'--client-root',str(app),'--output',str(app)],capture_output=True,text=True)
            self.assertNotEqual(result.returncode,0)
            self.assertIn('output must be separate from original client',result.stderr)
            self.assertEqual(list(app.iterdir()),[])
if __name__=='__main__':unittest.main()
