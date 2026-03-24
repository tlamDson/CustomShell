# CustomShell

Small Unix-like shell in C.

## Main Features

1. Batch mode: run shell with a script file argument.
2. Logical operators: short-circuit execution for `&&` and `||`.
3. Aliases: define, override, recursive expansion, loop protection.
4. History navigation: `Up`/`Down` arrows recall previous/next commands.

Other supported behavior:

1. `cd`, `PWD/OLDPWD`, `jobs`, pipes, redirection, background jobs, Ctrl+C/Ctrl+D handling.

## Requirements

1. Linux environment (WSL recommended on Windows)
2. `gcc`, `make`, `python3`

Optional:

1. `valgrind`

## Setup And Build

If you are in Windows PowerShell:

```bash
wsl
cd /mnt/d/Desktop/CustomShell\ -\ Copy
make -B
```

If you are already inside WSL terminal, do not type `wsl` again:

```bash
cd /mnt/d/Desktop/CustomShell\ -\ Copy
make -B
```

Run shell:

```bash
./bin/customShell
```

Run batch mode:

```bash
./bin/customShell /path/to/script.txt
```

## Manual Tests

### Batch mode

```bash
cat >/tmp/batch_test.txt <<"EOF"
echo one

echo two
echo three
EOF
./bin/customShell /tmp/batch_test.txt
```

Expected: prints `one`, `two`, `three` and exits.

### Logical operators

```text
shell208> true && echo AND_OK
shell208> false && echo SHOULD_NOT_PRINT
shell208> true || echo SHOULD_NOT_PRINT
shell208> false || echo OR_OK
shell208> false && echo NOPE || echo Done
```

Expected: `AND_OK`, `OR_OK`, `Done`; no `SHOULD_NOT_PRINT`, no `NOPE`.

### Aliases

```text
shell208> alias l='echo LS -l'
shell208> l -a
shell208> alias a='b'
shell208> alias b='a'
shell208> a
```

Expected: `LS -l -a` is printed; alias loop is detected and shell does not hang.

### CD and env vars

```text
shell208> env | grep PWD
shell208> cd /tmp
shell208> env | grep PWD
shell208> cd -
```

Expected: `PWD` and `OLDPWD` update correctly.

### Ctrl+C and background job

```text
shell208> sleep 5
(press Ctrl+C)
shell208> sleep 3 &
shell208> jobs
shell208> sleep 4
shell208> jobs
```

Expected: foreground sleep is interrupted; background sleep shows `Running` then `Done`.

### Command history (Up/Down arrows)

```text
shell208> echo first
shell208> echo second
(press Up) -> shows: echo second
(press Up) -> shows: echo first
(press Down) -> shows: echo second
```

Expected: Up/Down cycles command history without retyping.

## Automated Tests

Run all test files:

```bash
python3 tests/python/test_all.py
```

Or discovery mode:

```bash
python3 -m unittest discover -s tests/python -p "test_*.py" -v
```

Run each feature test file:

1. Batch mode: `python3 tests/python/test_batch_mode.py`
2. Logical operators: `python3 tests/python/test_logical_operations.py`
3. Aliases: `python3 tests/python/test_aliases.py`
4. CD, PWD, OLDPWD: `python3 tests/python/test_cd_pwd_oldpwd.py`
5. Ctrl+C behavior: `python3 tests/python/test_ctrl_c.py`
6. Background jobs: `python3 tests/python/test_background_jobs.py`

## Clean

```bash
make clean
```
