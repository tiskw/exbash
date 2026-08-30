////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: grid_picker.cxx                                                                    ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "grid_picker.hxx"

// Include custom headers.
#include "grid_model.hxx"
#include "grid_window.hxx"
#include "utils.hxx"


namespace
{
    Vector<String> load_history_tokens(const Path& path)
    {   // {{{

        constexpr auto is_history_timestamp = [](StringView line) -> bool
        // Check if a line from the history file is a timestamp line (starts with '#' followed by digits).
        //
        // [Args]
        //   line (StringView): [IN] The line to check.
        //
        // [Returns]
        //   (bool): True if the line is a timestamp line, false otherwise.
        {
            return line.size() > 1 && line.front() == '#' &&
                   std::all_of(line.begin() + 1, line.end(), [](unsigned char character) {
                       return character >= '0' && character <= '9';
                   });
        };

        //
        if (not stdfs::exists(path))
            return {};

        Vector<String> result;
        for (const String& line : read_lines(path))
            if (not is_history_timestamp(line))
                for (const StringView token : shlex_split(line))
                    result.emplace_back(String(token));

        return result;

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

String run_grid_picker(StringView lhs, const OmniPickerConfig& cfg, StringView user_input)
{   // {{{

    // Load the history tokens from the specified history file path.
    Vector<String> items = load_history_tokens(expand_tilde(cfg.path_history));

    // Initialize the filer state with the initial path.
    GridModel model(items);

    // Initialize the filer window with the specified width ratios and user input.
    GridWindow window(cfg.colors, user_input);

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
            return String(lhs) + join(model.get_selected());

        // Otherwise, process the key and update the model state.
        const String new_grep_str = model.process_key(key, window.get_grep_str(), window.get_grid_shape());
        window.set_grep_str(new_grep_str);
    }

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
