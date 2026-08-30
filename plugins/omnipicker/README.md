omnipicker: A file chooser plugin for ExBash
====================================================================================================

<p align="center">
  <img src="https://img.shields.io/badge/C++-23-blue?style=plastic" /> &nbsp;
  <img src="https://img.shields.io/badge/coverage-95.1%25-green?style=plastic" />
</p>

`omnipicker` is an ExBash plugin that displays a text-based file browser and writes the selected
paths to the destination specified by `--output`. ExBash then reads this output and inserts
the selected paths into the command buffer. `omnipicker` supports directory navigation,
file and directory previews, filtering, hidden-file display, and multiple selections.


Usage
----------------------------------------------------------------------------------------------------

`omnipicker` is normally launched using a key binding configured in ExBash. In the default ExBash
configuration, `Ctrl-F` is the trigger for `omnipicker`. The command-line interface of `omnipicker`
is as follows:

```console
omnipicker -l STR -r STR -o PATH [-c PATH] [-i STR]
omnipicker (-h|--help)
omnipicker (-v|--version)
```

### Required arguments

* `-l`, `--lhs`: The part of the string currently being edited that is to the left of the cursor.
* `-r`, `--rhs`: The part of the string currently being edited that is to the right of the cursor.
* `-o`, `--output`: The path to the plugin output file. If `stdout` or `STDOUT` is specified,
                    the result of the plugin is written to standard output.

### Optional arguments

* `-c`, `--config`: Path to a TOML configuration file.
* `-i`, `--input`: Keystrokes to preload when `omnipicker` starts. The default value is an empty string.
                   This option is primarily useful for automated testing.
* `-h`, `--help`: Show the help message and exit.
* `-v`, `--version`: Show the version and exit.


Keyboard controls
----------------------------------------------------------------------------------------------------

### File browser

| Key                | Action                                                                          |
|--------------------|---------------------------------------------------------------------------------|
| `j`, Down          | Move the focus down one item.                                                   |
| `k`, Up            | Move the focus up one item.                                                     |
| `Ctrl-F`           | Move the focus down 10 items.                                                   |
| `Ctrl-B`           | Move the focus up 10 items.                                                     |
| `0`                | Move the focus to the first item.                                               |
| `G`                | Move the focus to the last item.                                                |
| `l`, Right         | Enter the focused directory.                                                    |
| `h`, Left, `-`     | Move to the parent directory.                                                   |
| Space              | Select or deselect the focused item, then move the focus down.                  |
| `.`                | Toggle the display of hidden files.                                             |
| `/`                | Open the filter input window.                                                   |
| `Ctrl-P`           | Open the expanded preview window.                                               |
| Enter              | Confirm the selection and write the selected path(s) to the output destination. |
| `q`, `Q`, `Ctrl-D` | Cancel without changing the ExBash command buffer.                              |

### Filter window

Users can filter the displayed items by typing text in the filter window. The filter performs
a case-sensitive substring match on file and directory names. By default, the filter window
can be opened by pressing `/` in the file browser. The following table lists the key bindings
available in the filter window:

| Key             | Action                                                     |
|-----------------|------------------------------------------------------------|
| Enter           | Close the filter input window and keep the current filter. |
| Backspace       | Remove the last UTF-8 character from the filter.           |
| Other text keys | Append the entered character to the filter.                |

### Preview window

Users can inspect file contents in an expanded preview window that is larger than the preview area
on the left side of the file browser. By default, the preview window can be opened by pressing
`Ctrl-P` in the file browser. The following table lists the key bindings available in the preview
window:

| Key           | Action                             |
|---------------|------------------------------------|
| `j`, Down     | Scroll down by one line.           |
| `k`, Up       | Scroll up by one line.             |
| `q`, `Ctrl-P` | Close the expanded preview window. |


Configuration
----------------------------------------------------------------------------------------------------

`omnipicker` reads only the `[PLUGIN_omnipicker]` section of a TOML configuration file and ignores
all other sections. Therefore, you can place the `omnipicker` settings directly in the main ExBash
configuration file.

`omnipicker` searches for a configuration file in the following order and uses the first one it finds:
1. The path supplied with `--config`
2. `~/.config/exbash/config.toml`
3. `~/.local/share/exbash/default/config.toml`

If no configuration file is found, the built-in defaults are used. If the selected TOML file cannot
be parsed, an error message is written to standard error, and the built-in defaults are used.

### Configuration items

| Item              | Default | Description                                                                                               |
|-------------------|--------:|-----------------------------------------------------------------------------------------------------------|
| `w1_ratio`        | `0.20`  | Width ratio of the parent-directory panel.                                                                |
| `w2_ratio`        | `0.45`  | Width ratio of the current-directory panel.                                                               |
| `preview_cmd_txt` | `""`    | Command used to preview text files. `{path}` is replaced with the shell-quoted path of the target file.   |
| `preview_cmd_bin` | `""`    | Command used to preview binary files. `{path}` is replaced with the shell-quoted path of the target file. |

### Preview command examples

```toml
# Use the "batcat" command to preview text files. Preview is limited to the first 64 lines for performance.
preview_cmd_txt = "batcat --color=always --paging=never --style=plain --theme=ansi --line-range=:64 {path}"

# Use file to display a brief description of a binary file.
preview_cmd_bin = "file {path}"
 
# Alternatively, use xxd to display the first 64 lines of a hexadecimal dump.
# preview_cmd_bin = "xxd {path} | head -n 64"
```


Security considerations
----------------------------------------------------------------------------------------------------

* Preview commands are read from the configuration file and executed by the shell with the privileges
  of the current user. Use only trusted commands and trusted configuration files.
* The `{path}` placeholder in a preview command is replaced with the shell-quoted path of the target
  file, but all other text in the command is interpreted as shell syntax.


Dependencies
----------------------------------------------------------------------------------------------------

`omnipicker` is implemented in C++ and uses the following third-party libraries:
* [ncurses](https://invisible-island.net/ncurses/): A terminal control library for text-based user interfaces.
* [toml++](https://github.com/marzer/tomlplusplus): A header-only TOML parser.


License
----------------------------------------------------------------------------------------------------

`omnipicker` is distributed as part of ExBash and is licensed under the same terms.

