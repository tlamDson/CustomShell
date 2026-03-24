import unittest

from common import run_shell


class BackgroundJobsTests(unittest.TestCase):
    def test_sleep_background_running_then_done(self):
        script = "sleep 3 &\njobs\nsleep 4\njobs\nexit\n"
        result = run_shell(script, timeout=15)
        self.assertEqual(result.returncode, 0)

        text = result.stdout + result.stderr
        self.assertIn("Running sleep", text)
        self.assertIn("Done sleep", text)


if __name__ == "__main__":
    unittest.main(verbosity=2)
