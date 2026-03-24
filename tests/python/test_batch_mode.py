import os
import stat
import tempfile
import unittest

from common import run_shell, combined_output


class BatchModeTests(unittest.TestCase):
    def test_basic_three_commands(self):
        with tempfile.NamedTemporaryFile("w", delete=False) as script:
            script.write("echo one\necho two\necho three\n")
            script_path = script.name

        try:
            result = run_shell(args=[script_path])
            self.assertEqual(result.returncode, 0)
            out = combined_output(result)
            self.assertIn("one", out)
            self.assertIn("two", out)
            self.assertIn("three", out)
        finally:
            os.unlink(script_path)

    def test_ignores_empty_lines(self):
        with tempfile.NamedTemporaryFile("w", delete=False) as script:
            script.write("\n   \n\techo ok\n\n")
            script_path = script.name

        try:
            result = run_shell(args=[script_path])
            self.assertEqual(result.returncode, 0)
            out = combined_output(result)
            self.assertIn("ok", out)
            self.assertNotIn("Command not found", out)
        finally:
            os.unlink(script_path)

    def test_last_line_without_newline(self):
        with tempfile.NamedTemporaryFile("w", delete=False) as script:
            script.write("echo last_line")
            script_path = script.name

        try:
            result = run_shell(args=[script_path])
            self.assertEqual(result.returncode, 0)
            self.assertIn("last_line", combined_output(result))
        finally:
            os.unlink(script_path)

    def test_permission_denied(self):
        with tempfile.NamedTemporaryFile("w", delete=False) as script:
            script.write("echo should_not_run\n")
            script_path = script.name

        try:
            os.chmod(script_path, 0)
            result = run_shell(args=[script_path])
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Permission denied", result.stderr)
        finally:
            os.chmod(script_path, stat.S_IRUSR | stat.S_IWUSR)
            os.unlink(script_path)


if __name__ == "__main__":
    unittest.main(verbosity=2)
