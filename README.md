# dcsh

A small Unix-style shell written in C to make command execution, process creation, and shell history easier to understand from source.

## What This Is

`dcsh` is a compact command-line shell for running simple commands on macOS, Linux, and WSL. The goal is not to replace a full shell like `bash` or `zsh`, but to build a readable command runner where the core Unix process model is visible in the code.

Full shells contain decades of features, configuration behavior, scripting rules, and edge cases. This project keeps the scope small so the important mechanics are easier to follow: reading commands, launching programs, waiting for child processes, redirecting files, connecting commands with a pipe, and storing recent command history.

## Technical Highlights

* Uses a REPL loop to continuously read and process user commands.
* Executes external commands with `fork`, `execvp`, and `waitpid`.
* Keeps parsing intentionally small and readable instead of implementing a full shell grammar.
* Supports basic input and output redirection with `open`, `dup2`, and `close`.
* Supports one command pipe with `pipe`, `fork`, and `dup2`.
* Runs built-in commands like `cd`, `pwd`, and `help` in the parent shell process.
* Plans support for command history.

## Demo

Initial scaffold:

```text
$ make
$ ./dcsh
dcsh> echo hello
hello
dcsh> echo hello > out.txt
dcsh> cat < out.txt
hello
dcsh> echo hello | wc -c
6
dcsh> pwd
/path/to/current/directory
dcsh> help
dcsh built-ins:
  cd [directory]  change the shell working directory
  pwd             print the shell working directory
  help            show this help message
  exit            exit dcsh
dcsh> exit
```

A screenshot or terminal recording will be added after the v1 shell features are complete.

## Build & Run

Build the shell:

```bash
make
```

Run it:

```bash
./dcsh
```

Clean build files:

```bash
make clean
```

This project targets POSIX-style systems. It is intended to run on macOS, Linux, or Windows through WSL.

## Design Decisions

I chose a small C implementation because the project is mainly about understanding the mechanics behind a shell, not hiding them behind libraries. The shell starts with a simple REPL and grows feature by feature, which keeps each part easier to test and reason about.

The parser is intentionally limited in v1. It focuses on whitespace-separated commands, basic redirection, and one pipe instead of trying to behave like a complete production shell. That keeps the project readable while still covering the core systems programming ideas: processes, file descriptors, and command execution.

The planned history feature will use a fixed-size circular buffer. This avoids dynamic data structures for a feature that naturally has a fixed limit, and it makes the memory behavior easier to inspect.

## What I Learned

This project is a practical look at how shells connect user input to operating system behavior. The main focus is understanding how a command becomes a child process, how the parent process waits for it, how file descriptors can change where input and output go, and why built-ins like `cd` must run in the parent shell process.

Future work could add quoted strings, append redirection, multiple pipes, better error messages, and a more complete parser. The current goal is to keep the shell small enough that each feature can be understood directly from the source.
