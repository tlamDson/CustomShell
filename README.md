# CustomShell

A small Unix-like shell written in C.

## Features

- Execute commands with `fork` and `execv`
- Custom `$PATH` resolution via `getenv("PATH")`
- Input/output redirection
- Pipes
- Background jobs (`&`)
- Basic job control list (`jobs`)
- Signal handling (`SIGINT`, `SIGCHLD`)
- Cleanup all jobs on shell exit to avoid leaks

## Requirements

- Linux environment (recommended: WSL on Windows)
- `gcc`
- `make`
- `valgrind` (for memory-leak testing)

## Setup (WSL)

From Windows PowerShell:

```bash
wsl
```

Inside WSL, go to the project folder:

```bash
cd /mnt/d/Desktop/CustomShell\ -\ Copy
```

Install build tools and Valgrind:

```bash
sudo apt update
sudo apt install -y build-essential valgrind
```

If `apt` shows lock/permission errors, make sure you used `sudo`.

If `apt` shows `404 Not Found`, refresh package metadata:

```bash
sudo apt clean
sudo rm -rf /var/lib/apt/lists/*
sudo apt update --fix-missing
```

## Build

Use the Makefile:

```bash
make -B
```

Output binary:

```bash
bin/customShell
```

## Run

```bash
./bin/customShell
```

Example:

```text
shell208> ls
shell208> jobs
shell208> Ctrl+D
```

You can still type `exit`, but pressing Ctrl+D on an empty prompt will exit the shell.

## Test

Quick non-interactive test:

```bash
printf "exit\n" | ./bin/customShell
```

Automated TDD suite (Python, split by feature):

```bash
python3 -m unittest discover -s tests/python -p "test_*.py" -v
```

Single entrypoint file (run all Python tests):

```bash
python3 tests/python/test_all.py
```

PATH resolution test:

```bash
cat > /tmp/shell_path_test_input.txt <<"EOF"
pwd
/bin/echo ABS_OK
abcxyz123
exit
EOF
./bin/customShell < /tmp/shell_path_test_input.txt
```

Expected:

- `pwd` runs normally (found through `$PATH`)
- `/bin/echo ABS_OK` runs directly (absolute path)
- Unknown command prints `Command not found: ...`

Parser note:

- Redirection tokens must be separated by spaces, for example: `ls > out.txt`

## CD Test Suite

Run these tests inside the shell to validate `cd` behavior.

### 1. Basic functionality

```text
shell208> pwd
shell208> cd /tmp
shell208> pwd
shell208> cd ..
shell208> pwd
shell208> cd
shell208> pwd
shell208> cd ~
shell208> pwd
```

Expected:

- `pwd` changes after `cd /tmp` and `cd ..`
- `cd` (no args) returns to `$HOME`
- `cd ~` also returns to `$HOME`

### 2. Edge cases

```text
shell208> cd /tmp
shell208> cd -
shell208> cd does_not_exist_123
shell208> cd /root
```

Expected:

- `cd -` switches to previous directory and prints that path
- Non-existing path shows `No such file or directory`
- Permission-restricted path shows `Permission denied`

### 3. Environment variable checks

```text
shell208> env | grep PWD
shell208> cd /tmp
shell208> env | grep PWD
```

Expected:

- `PWD` updates to new directory
- `OLDPWD` keeps previous directory

### 4. Memory-leak check (Valgrind)

```bash
printf "cd /tmp\ncd /\ncd -\ncd ~\ncd does_not_exist_123\nexit\n" | valgrind --leak-check=full --show-leak-kinds=all ./bin/customShell
```

Expected:

- `All heap blocks were freed -- no leaks are possible`
- `ERROR SUMMARY: 0 errors from 0 contexts`

## Valgrind Memory-Leak Testing

Run with full leak diagnostics:

```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/customShell
```

To test the background-job exit scenario:

```text
shell208> sleep 30 &
shell208> exit
```

Automated check:

```bash
printf "sleep 5 &\nexit\n" | valgrind --leak-check=full --show-leak-kinds=all ./bin/customShell
```

Expected after cleanup logic:

- No `definitely lost` bytes
- No `indirectly lost` bytes

## Clean

Remove build artifacts:

```bash
make clean
```

## Author

- Lam Pham
