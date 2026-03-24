import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SHELL_BIN = ROOT / "bin" / "customShell"


def run_shell(input_text=None, args=None, timeout=12):
    if args is None:
        args = []

    return subprocess.run(
        [str(SHELL_BIN), *args],
        input=input_text,
        capture_output=True,
        text=True,
        cwd=ROOT,
        timeout=timeout,
    )


def combined_output(result):
    return result.stdout + result.stderr
