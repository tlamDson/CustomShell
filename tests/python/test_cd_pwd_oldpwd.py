import re
import unittest

from common import run_shell


class CdPwdOldpwdTests(unittest.TestCase):
    def test_pwd_oldpwd_updates_after_cd(self):
        result = run_shell("env | grep PWD\ncd /tmp\nenv | grep PWD\nexit\n")
        self.assertEqual(result.returncode, 0)
        text = result.stdout + result.stderr
        self.assertRegex(text, r"PWD=.*")
        self.assertRegex(text, r"OLDPWD=.*")
        self.assertIn("PWD=/tmp", text)

    def test_cd_dash_switches_and_prints_previous_dir(self):
        result = run_shell("pwd\ncd /tmp\ncd -\npwd\nexit\n")
        self.assertEqual(result.returncode, 0)
        lines = [line.strip() for line in (result.stdout + result.stderr).splitlines() if line.strip()]
        self.assertGreaterEqual(len(lines), 3)
        initial_pwd = lines[0]
        printed_by_cd_dash = lines[1]
        pwd_after_cd_dash = lines[2]

        self.assertEqual(printed_by_cd_dash, initial_pwd)
        self.assertEqual(pwd_after_cd_dash, initial_pwd)


if __name__ == "__main__":
    unittest.main(verbosity=2)
