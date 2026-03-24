import unittest

from common import run_shell


class LogicalOperationsTests(unittest.TestCase):
    def test_and_success(self):
        result = run_shell("true && echo AND_OK\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertIn("AND_OK", result.stdout + result.stderr)

    def test_and_failure(self):
        result = run_shell("false && echo SHOULD_NOT_PRINT\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertNotIn("SHOULD_NOT_PRINT", result.stdout)

    def test_or_success(self):
        result = run_shell("true || echo SHOULD_NOT_PRINT\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertNotIn("SHOULD_NOT_PRINT", result.stdout)

    def test_or_failure(self):
        result = run_shell("false || echo OR_OK\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertIn("OR_OK", result.stdout + result.stderr)

    def test_complex_chain(self):
        result = run_shell("false && echo NOPE || echo Done\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertNotIn("NOPE", result.stdout)
        self.assertIn("Done", result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main(verbosity=2)
