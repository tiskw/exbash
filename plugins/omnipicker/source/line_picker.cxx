////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: line_picker.cxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the header.
#include "line_picker.hxx"

// Include the headers of STL.
#include <algorithm>
#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>

// Include the POSIX headers.
#include <unistd.h>

// Include custom headers.
#include "config.hxx"
#include "line_model.hxx"
#include "line_window.hxx"
#include "strextra.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// File-local helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    Vector<String> line_picker_body(const Vector<String>& items, const OmniPickerConfig& cfg, StringView user_input)
    // Run the line picker TUI event loop and return the selected item(s).
    //
    // [Args]
    //   items      (const Vector<String>&)  : [IN] List of text items to display.
    //   cfg        (const OmniPickerConfig&): [IN] The configuration values.
    //   user_input (StringView)             : [IN] Optional string of keystrokes to simulate as input.
    //
    // [Returns]
    //   (Vector<String>): The selected items (empty if the user aborted).
    //
    {   // {{{

        // Initialize the filer state with the initial path.
        LineModel model(items);

        // Initialize the filer window with the specified width ratios and user input.
        LineWindow window(cfg.colors, user_input);

        while (true)
        {
            // Update the model state based on the current directory and user input.
            window.draw(model.get_contents_filt(), model.get_focus_filt());

            // Get the user input key.
            String key = window.getkey();

            // If the user input is Ctrl-D, exit the loop regardless of the window state.
            if (key == "^D") return {};

            // If the user input is exit key, return an empty vector (indicate cancellation).
            if ((key == "q") or (key == "Q")) return {};

            // If the user input is enter key, return the selected path(s).
            if ((key == "^M") or (key == "^J"))
                return model.get_selected();

            // Otherwise, process the key and update the model state.
            const String updated_grep_str = model.process_key(key, window.get_grep_str());
            window.set_grep_str(updated_grep_str);
        }

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

String run_env_picker(StringView lhs, const OmniPickerConfig& cfg, StringView user_input)
{   // {{{

    constexpr auto get_env_lines = [](void) -> Vector<String>
    // Get the environment variables as a vector of strings in the format "KEY = VALUE".
    //
    // [Returns]
    //   (Vector<String>): A vector of strings representing the environment variables.
    {
        Vector<String> env_lines;

        for (char** env = environ; *env != nullptr; ++env)
        {
            // Convert the C-style string to a StringView.
            StringView env_entry(*env);

            // Split the entry into key and value at the first '=' character.
            const String::size_type pos = env_entry.find('=');
            const StringView key = env_entry.substr(0, pos);
            const StringView val = env_entry.substr(pos + 1);

            // Format the key-value pair and add it to the vector.
            env_lines.push_back(std::format("{} = {}", key, val));
        }

        return env_lines;
    };

    // Collect the values of all environment variables into a vector.
    Vector<String> env_values = get_env_lines();
    std::sort(env_values.begin(), env_values.end());

    // Run the text chooser TUI and collect the selected environment variable(s).
    return String(lhs) + join(line_picker_body(env_values, cfg, user_input));

}   // }}}

String run_hst_picker(StringView lhs, const OmniPickerConfig& cfg, StringView user_input)
{   // {{{

    // Check if the history file exists, and throw an error if it does not.
    const Path path_hist = expand_tilde(cfg.path_history);

    // Read and reverse the history entries so the most recent appears first.
    Vector<String> histories;
    if (stdfs::exists(path_hist))
    {
        for (const String& line : read_lines(path_hist.c_str()))
        {
            // Strip extra whitespace from the line.
            const StringView line_stripped = strip(line);

            // Append to the histories vector if the line is not empty.
            if (not line_stripped.empty())
                histories.push_back(String(line_stripped));
        }

        // Reverse the histories vector (the most recent history entry appears first).
        std::reverse(histories.begin(), histories.end());
    }

    return String(lhs) + join(line_picker_body(histories, cfg, user_input));

}   // }}}

String run_pid_picker(StringView lhs, const OmniPickerConfig& cfg, StringView user_input)
{   // {{{

    // Run "ps aux" and collect the output lines (skipping the header line).
    const String ps_output = check_output(cfg.ps_command);
    Vector<String> lines;
    {
        for (const auto& [idx, line] : split_lines(ps_output))
        {
            // Skip the header line.
            if ((idx != 0) and not line.empty())
                lines.push_back(line);
        }
    }

    // Run the text chooser TUI and collect the selected process lines.
    const Vector<String> items = line_picker_body(lines, cfg, user_input);

    // Extract the PIDs from the selected lines.
    Vector<String> selected_pids;
    for (const String& item : items)
    {
        // Split the line into tokens based on whitespace.
        const Vector<StringView> tokens = shlex_split(item);

        // If there are enough tokens, extract the PID and print it.
        if (tokens.size() > static_cast<SizeType>(cfg.idx_pid_field))
            selected_pids.push_back(String(tokens[cfg.idx_pid_field]));
    }

    return String(lhs) + join(selected_pids);

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
