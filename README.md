# SimpleShell

**SimpleShell** is a basic shell implementation in C that can execute system commands, handle piped commands, maintain a command history, and log execution times. It is a lightweight, UNIX-like terminal emulator that mimics core shell behavior, making it a great educational project for learning about process control, piping, and system programming in C.

---

## 🔧 Features

* **Command Execution**: Runs any valid UNIX command.
* **Piped Commands**: Supports a chain of piped commands (e.g., `ls -l | grep txt | wc -l`).
* **Command History**: Maintains and displays a persistent history of commands in `history.txt`.
* **Execution Time Logging**: Logs the start time, end time, duration, and PID of each executed command.
* **Interrupt Handling**: Gracefully handles `Ctrl+C` (SIGINT) to display history before exiting.
* **Modular Codebase**: Organized into reusable and logically separated functions.

---

## 🧠 How It Works

### 1. Input Handling

The shell continuously prompts the user for input. On receiving a command:

* It checks for pipes (`|`) to determine if it’s a simple or piped command.
* The command is added to the history and then executed.

### 2. Command Execution

* **Simple Commands**: Forks a child process and uses `execvp` to run the command.
* **Piped Commands**: Splits commands using `|`, sets up pipes, and manages input/output redirection.

### 3. Logging & History

* Command execution time and PID are logged into `history.txt`.
* On typing `history` or pressing `Ctrl+C`, the command history is printed.

---

## 📁 File Structure

```bash
.
├── history.txt        # Automatically generated log of command executions
└── shell.c            # Main source code for the SimpleShell
```

---

## 🚀 Getting Started

### 🔨 Compilation

Compile the code using `gcc`:

```bash
gcc shell.c -o simpleshell
```

### ▶️ Usage

Run the shell:

```bash
./simpleshell
```

Now you can type UNIX commands just like a regular shell.

#### Example:

```bash
group50@SimpleShell $ ls -l
group50@SimpleShell $ history
group50@SimpleShell $ ls | grep c | wc -l
```

---

## 📌 Built-in Commands

| Command   | Description                      |
| --------- | -------------------------------- |
| `exit`    | Displays history and exits shell |
| `history` | Displays command history         |
| `Ctrl+C`  | Displays history and exits shell |

---

## 🧹 Cleanup

To clear previous history:

```bash
> history.txt
```

---

## 🧑‍💻 Contributors

* **Satvik Arya (2023493)** – Designed command parsing, single command execution, signal handling, and history management.
* **Sanjeev (2023483)** – Developed pipe execution logic, time tracking, and logging functionality.

---

## 📜 License

This project is intended for educational purposes only and is distributed under the MIT License.

