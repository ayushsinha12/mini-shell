# Mini Shell | Custom Unix Shell in C

A full-featured Unix-style shell written in C that implements tokenization, parsing, process control, and command execution from scratch. This project supports shell operators such as sequencing (`;`), input/output redirection (`<`, `>`), and pipes (`|`). It includes built-in commands, error handling, and a robust tokenizer that handles quoted arguments and special characters.

---

## Description

Mini Shell is a fully functional Unix-style shell built from scratch in C, implementing a tokenizer, parser, and command execution system. The shell supports both basic and advanced shell operations including command sequencing (`;`), input/output redirection (`<`, `>`), and piping (`|`). It features a custom-built tokenizer that processes raw command-line input into lexical tokens, supporting quoted strings and special shell characters.

The shell executes system commands, manages child processes using `fork` and `execvp`, and includes built-in commands such as `cd`, `source`, `prev`, and `help`. It handles user input interactively through a prompt loop, supporting graceful exit via `exit` or `Ctrl-D`. Error handling, memory management (validated via Valgrind), and command parsing follow a deterministic grammar-based design. The project was compiled and tested using a custom Makefile and evaluated against rigorous functional test cases.

---

## Features

- **Custom Tokenizer**  
  Parses command-line input into tokens including quoted strings and special shell characters.

- **Shell Execution Engine**  
  Runs system commands via child processes with correct I/O handling and error messaging.

- **Command Sequencing (`;`)**  
  Supports multiple commands in one line executed in sequence.

- **Input Redirection (`<`)**  
  Routes file input to stdin of a command.

- **Output Redirection (`>`)**  
  Writes stdout of a command to a specified file.

- **Pipe Operator (`|`)**  
  Connects output of one command to input of another.

- 🛠️ **Built-in Commands**  
  - `cd`: Change directory  
  - `source`: Run script files  
  - `prev`: Re-run the last command  
  - `help`: Show help for built-in commands  
  - `exit` / `Ctrl-D`: Exit the shell

- **Makefile Build System**  
  Includes targets to build, clean, and test both tokenizer and shell.

---

## 🚀 How to Run

1. **Clone the repository:**
   ```bash
   git clone https://github.com/ayushsinha12/mini-shell.git
   cd mini-shell

## Example Usage

```bash
shell $ echo Hello World
Hello World

shell $ whoami
ayush

shell $ echo Boston; echo San Francisco
Boston
San Francisco

shell $ sort < input.txt > sorted.txt

shell $ shuf -i 1-10 | sort -n | tail -5

shell $ prev
# re-executes previous command

shell $ cd src
shell $ help
# displays all built-in commands
```

## File Structure
```bash
├── shell.c           # Main shell implementation
├── tokens.c/.h       # Tokenizer logic
├── vect.c/.h         # Vector helper for token lists
├── Makefile          # Build & test automation
├── test/             # Tokenizer and shell tests
```