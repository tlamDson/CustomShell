import os
import signal
import subprocess
import time
import unittest

from common import ROOT, SHELL_BIN


class CtrlCTests(unittest.TestCase):
    def test_ctrl_c_interrupts_foreground_sleep(self):
        proc = subprocess.Popen(
            [str(SHELL_BIN)],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            cwd=ROOT,
            preexec_fn=os.setsid,
        )

        try:
            proc.stdin.write("sleep 5\n")
            proc.stdin.flush()
            time.sleep(0.5)

            os.killpg(proc.pid, signal.SIGINT)
            time.sleep(0.4)

            proc.stdin.write("echo AFTER_INT\nexit\n")
            proc.stdin.flush()

            out, err = proc.communicate(timeout=8)
            self.assertEqual(proc.returncode, 0)
            self.assertIn("AFTER_INT", out + err)
        finally:
            if proc.poll() is None:
                proc.kill()


if __name__ == "__main__":
    unittest.main(verbosity=2)
