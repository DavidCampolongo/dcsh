# dcsh — a small POSIX shell in C

[![Build and test](https://github.com/DavidCampolongo/dcsh/actions/workflows/build.yml/badge.svg)](https://github.com/DavidCampolongo/dcsh/actions/workflows/build.yml)

A C11 command interpreter that launches processes, redirects input/output, and connects two commands with a pipe. Built to understand how a shell turns a line of text into processes and file descriptors.

## Try it

Requires a C compiler and Make on Linux or macOS. No external libraries.

```sh
make
make test
./dcsh
```

Inside the shell:

```text
dcsh> echo hello > out.txt
dcsh> cat < out.txt
hello
dcsh> cat out.txt | wc -c
6
dcsh> cd /tmp
dcsh> pwd
/tmp
dcsh> exit
```

`out.txt` is created in the directory where you started the shell. `make clean` removes the compiled shell and test runner.

## How it works

| Component | Responsibility |
| --- | --- |
| [Parser](src/parser.c) | Tokenize commands and `<`, `>`, `|`, including operators without surrounding spaces. |
| [Executor](src/executor.c) | Use `fork`, `execvp`, and `waitpid` to run commands; connect descriptors with `open`, `dup2`, and `pipe`. |
| [Shell loop](src/shell.c) | Read commands and run `cd`, `pwd`, `help`, and `exit` in the parent process. |

`cd` must run in the parent: changing a child's working directory would not change the shell's directory. For a pipeline, both children are started before the parent waits; unused pipe ends are closed so the reader can reach EOF.

## Verification

[The test runner](tests/test_dcsh.c) exercises compact operator parsing, output redirection, combined input/output redirection, and a single pipeline using temporary files. GitHub Actions builds and runs the tests on Linux and macOS for pushes and pull requests.

## Scope

This is a learning implementation, not a replacement for an interactive system shell. It supports one pipe and basic redirection. Quoting, variable expansion, append redirection, background jobs, command history, and full interactive signal handling are not implemented.
