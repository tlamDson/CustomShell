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
shell208> exit
```

## Test

Quick non-interactive test:

```bash
printf "exit\n" | ./bin/customShell
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
