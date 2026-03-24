#!/usr/bin/env python3
import pathlib
import sys
import unittest


def main():
    test_dir = pathlib.Path(__file__).resolve().parent
    suite = unittest.defaultTestLoader.discover(str(test_dir), pattern="test_*.py")
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())
