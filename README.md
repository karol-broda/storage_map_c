# Directory Navigator

Directory Navigator is a terminal-based application for navigating and managing directories with a visual interface.

## Features

- Navigate directories and view sizes
- Toggle file display
- Delete files or directories
- Reveal in Finder
- Responsive UI with `ncurses`

## Requirements

- GCC compiler
- `ncurses` library
- Root privileges

## Installation

1. Clone the repository:

    ```bash
    git clone https://github.com/karol-broda/storage_map_c.git
    cd directory-navigator
    ```

2. Compile the project:

    ```bash
    make
    ```

## Usage

Run the application with a directory as an argument:

```bash
sudo ./dir_navigator /path/to/directory
```

## Key Bindings

- `Arrow keys`: Navigate
- `Enter`: Select
- `b`: Go back
- `t`: Toggle file display
- `x`: Delete (with confirmation)
- `r`: Reveal in Finder
- `h`: Help
- `q`: Quit

## Code Overview

### main.c

- Initializes `ncurses`
- Sets up signal handlers
- Starts directory navigation

### terminal.c

- Retrieves terminal size
- Manages resize and cleanup

### node.c

- Manages directory and file nodes
- Adds paths and traverses directories
- Deletes directories and files

### display.c

- Displays directories and files
- Handles user input

### directory.c

- Manages directory operations and threading

## License

This project is licensed under the MIT License.

---

For questions or contributions, visit the GitHub repository or contact me at karol.broda@hotmail.com.