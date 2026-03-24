import unittest

from common import run_shell


class AliasTests(unittest.TestCase):
    def test_simple_replacement(self):
        result = run_shell("alias c='echo CLEAR'\nc\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertIn("CLEAR", result.stdout + result.stderr)

    def test_preserves_arguments(self):
        result = run_shell("alias l='echo LS -l'\nl -a\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertIn("LS -l -a", result.stdout + result.stderr)

    def test_override(self):
        result = run_shell("alias a='echo ONE'\nalias a='echo TWO'\na\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertNotIn("ONE\n", result.stdout)
        self.assertIn("TWO", result.stdout + result.stderr)

    def test_recursive_resolution(self):
        result = run_shell("alias a='b'\nalias b='echo B_OK'\na\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertIn("B_OK", result.stdout + result.stderr)

    def test_infinite_loop_protection(self):
        result = run_shell("alias a='b'\nalias b='a'\na\nexit\n", timeout=5)
        self.assertEqual(result.returncode, 0)
        self.assertIn("alias: expansion loop detected", result.stderr)


if __name__ == "__main__":
    unittest.main(verbosity=2)
