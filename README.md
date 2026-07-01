# dcsh

A small Unix-style shell written in C11 that reads commands, parses shell operators, launches external programs, supports basic redirection, connects one pipeline, and runs built-in commands that must execute inside the parent shell process.

I built this project because I wanted to better understand what happens between typing a command and the operating system actually running a program. A shell sits close to the boundary between user programs, processes, file descriptors, and the kernel. Rebuilding a small version of that helped me study process control, Unix dataflow, parent/child process behavior, and how operating systems expose low-level execution primitives.

This project is intentionally not a full replacement for `bash` or `zsh`. The goal is a compact, readable implementation of core systems programming ideas: parsing input, creating processes, redirecting file descriptors, coordinating a pipe, and handling built-ins that affect shell state.

## What This Project Demonstrates

* C11 programming
* POSIX systems programming
* Modular source/header organization
* REPL-style command loop
* Command parsing and tokenization
* Process creation with `fork`
* Program execution with `execvp`
* Child-process synchronization with `waitpid`
* File descriptor control with `open`, `dup2`, and `close`
* Input and output redirection
* Inter-process communication with `pipe`
* Single-pipeline command execution
* Parent-process built-ins such as `cd`
* Makefile-based build workflow
* Basic C test coverage without CMake or external frameworks

## System Overview

```text
User command
        |
        v
Shell REPL
        |
        v
Tokenizer / parser
        |
        v
Command dispatcher
        |
        +-----------------------------+
        |                             |
        v                             v
Built-in command              External command
parent shell process          child process
        |                             |
        v                             v
cd / pwd / help / exit        fork / execvp / waitpid
                                      |
                                      v
                         redirection / pipe setup
```

The program is intentionally split into small components:

* `shell.c` owns the interactive loop, handles `exit`, and dispatches built-ins before external execution.
* `parser.c` converts a command line into tokens, including operators such as `<`, `>`, and `|`.
* `executor.c` launches external commands, applies redirection, creates one pipeline, and waits for child processes.
* `main.c` starts the shell and converts shell status into the program exit code.

## Supported Commands

### External Command Execution

External programs are executed using the standard Unix process model:

```text
parent shell
    |
    v
fork child
    |
    v
execvp target program
    |
    v
parent waits with waitpid
```

Example:

```text
dcsh> echo hello
hello
```

### Output Redirection

The shell supports `cmd > file` by opening the target file and redirecting standard output with `dup2`.

```text
dcsh> echo hello > out.txt
```

### Input Redirection

The shell supports `cmd < file` by opening the input file and redirecting standard input with `dup2`.

```text
dcsh> cat < out.txt
hello
```

### Single Pipe

The shell supports one pipe using `cmd1 | cmd2`. It creates a pipe, forks both commands, connects the left command's standard output to the right command's standard input, closes unused file descriptors, and waits for both child processes.

```text
dcsh> echo hello | wc -c
6
```

### Built-ins

Some commands must run in the shell process itself instead of a forked child. `cd` is the main example: if a child process changed directories, the child would exit and the shell would stay in the same directory.

| Command | Description |
| --- | --- |
| `cd [directory]` | Change the shell working directory |
| `pwd` | Print the shell working directory |
| `help` | Print built-in command help |
| `exit` | Exit the shell |

## Project Structure

```text
include/
  dcsh.h
  executor.h
  parser.h

src/
  executor.c
  main.c
  parser.c
  shell.c

tests/
  test_dcsh.c

Makefile
README.md
```

## Build and Run

This project targets POSIX-style systems such as Linux, macOS, and Windows through WSL.

Build the shell:

```bash
make
```

Run the shell:

```bash
./dcsh
```

Clean build artifacts:

```bash
make clean
```

## Tests

The test target builds and runs a small C test program. It uses standard C and POSIX APIs, so there is no CMake setup or external testing framework.

```bash
make test
```

The test runner covers:

* Parser handling of compact operators such as `cat<input|wc>output`
* Output redirection with `>`
* Input and output redirection with `<` and `>`
* Single-pipe execution with `|`

## Example Session

```text
$ make
$ ./dcsh
dcsh> pwd
/path/to/project
dcsh> echo hello > out.txt
dcsh> cat < out.txt
hello
dcsh> echo hello | wc -c
6
dcsh> help
dcsh built-ins:
  cd [directory]  change the shell working directory
  pwd             print the shell working directory
  help            show this help message
  exit            exit dcsh
dcsh> exit
```

## Current Limits

The parser is intentionally small and does not try to implement a complete production shell grammar.

Current limitations:

* No quoted string parsing
* No environment variable expansion
* No append redirection with `>>`
* No multiple-command pipelines
* No background jobs
* No command history yet

## Future Improvements

Possible next steps:

* Add quoted string support
* Add append redirection with `>>`
* Add multiple pipes
* Add command history with a fixed-size circular buffer
* Add more built-ins such as `history` or `export`
* Improve parser diagnostics for invalid syntax
* Add signal handling for interactive use
* Expand tests around syntax errors and child-process failures
