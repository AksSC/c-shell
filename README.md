# Custom Shell - mysh

## Overview
This project is a custom shell implementation in C. It replicates many features of a Unix shell while adding some custom commands. The project emphasizes modular code design, ensuring clarity and maintainability. The shell supports fundamental system commands, background processes, process management, I/O redirection, and piping, among other functionalities.

## Features
### 1. **Shell Prompt**
- Displays the prompt in the format: `<Username@SystemName:~>`
- Supports dynamic updates when the working directory changes
- Shows absolute paths when outside the home directory

### 2. **Command Execution**
- Supports executing system commands like `ls`, `cat`, `echo`, `vim`, etc.
- Supports running multiple commands separated by `;` or `&`
- Background execution using `&` (prints PID of background process)
- Error handling for invalid commands

### 3. **Custom Commands**
- `hop <directory>`: Changes the directory similar to `cd`, with support for `.` `..` `~` and `-` shortcuts.
- `reveal [-a|-l] <path>`: Lists files and directories with optional detailed (`-l`) and hidden (`-a`) views.
- `log`: Stores command history across sessions (up to 15 entries), with options to execute (`log execute <index>`) or clear (`log purge`) history.
- `proclore [pid]`: Displays information about a process, including PID, state, process group, memory usage, and executable path.
- `seek [-d|-f|-e] <name> [directory]`: Searches for files/directories with optional filters (`-d` for directories, `-f` for files, `-e` for execution or directory change if single match is found).

### 4. **Process Management**
- Executes system commands in the foreground and background
- Tracks background processes, printing exit messages when they terminate
- `activities`: Lists all processes spawned by the shell with their states
- `ping <pid> <signal_number>`: Sends signals to a process
- `fg <pid>`: Brings a background process to the foreground
- `bg <pid>`: Resumes a stopped background process

### 5. **I/O Redirection & Piping**
- Supports input (`<`), output (`>`), and append (`>>`) redirection
- Handles multiple pipes (`|`) for chaining commands
- Works seamlessly with both built-in and system commands
- Allows combined use of redirection and piping

### 6. **.myshrc Configuration**
- Supports aliasing custom shortcuts for commands (e.g., `alias home = hop ~`)

### 7. **Networking & Extras**
- `iMan <command>`: Fetches online manual pages for commands using HTTP requests
- `neonate -n <time>`: Prints the most recently created process ID every `<time>` seconds until `x` is pressed

## Installation
1. Clone the repository:
   ```sh
   git clone https://github.com/AksSC/c-shell
   cd c-shell
   ```
2. Compile the shell:
   ```sh
   make
   ```
3. Run the shell:
   ```sh
   ./myshell
   ```

## Usage
### Running the Shell
Simply run the executable:
```sh
./myshell
```
The shell prompt will appear, allowing you to enter commands.

### Example Commands
```sh
<JohnDoe@SYS:~> hop test
/home/johndoe/test

<JohnDoe@SYS:~/test> reveal -l
-rw-r--r-- 1 user group 1024 file.txt

<JohnDoe@SYS:~/test> echo "Hello World" > output.txt

<JohnDoe@SYS:~/test> cat < output.txt | wc > count.txt

<JohnDoe@SYS:~/test> log
reveal -l
echo "Hello World" > output.txt

<JohnDoe@SYS:~/test> log execute 1
# Executes `reveal -l`
```

## Error Handling
- Invalid commands print a suitable error message.
- Handles missing files for input redirection gracefully.
- Invalid pipes return "Invalid use of pipe".
- Background process terminations are notified automatically.

## Known Limitations
- Background execution is not implemented for custom commands (e.g., `hop`, `reveal`). 
- Does not support multi-word aliases in `.myshrc`.

## Author
- **Aks Kanodia**
