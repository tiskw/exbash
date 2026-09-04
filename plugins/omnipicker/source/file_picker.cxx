////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: file_picker.cxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the header.
#include "file_picker.hxx"

// Include custom headers.
#include "file_model.hxx"
#include "file_window.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

String run_file_picker(StringView lhs, const OmniPickerConfig& cfg, StringView user_input)
{   // {{{

    // Extract the initial directory from the last token of the lhs string.
    const auto [lhs_rest, path_root] = get_valid_path_of_last_token(lhs);

    // Canonicalize the initial path to resolve any symlinks and get an absolute path.
    std::error_code ec;
    const Path path_resolved = stdfs::canonical(path_root, ec);

    // Initialize the filer state with the initial path.
    FileModel model(ec ? path_root : path_resolved, cfg);

    // Initialize the filer window with the specified width ratios and user input.
    FileWindow window(cfg.w1_ratio, cfg.w2_ratio, cfg.colors, user_input);

    while (true)
    {
        // Get the preview lines for the currently selected item if not empty.
        Vector<String> preview_lines;
        if (not model.get_item().left.empty())
            preview_lines = model.get_preview(model.get_item().left);

        // Update the model state based on the current directory and user input.
        window.draw(model.get_contents_filt(), model.get_focus_filt(),
                    model.get_contents_prev(), model.get_focus_prev(),
                    preview_lines);

        // Get the user input key.
        String key = window.getkey();

        // If the user input is Ctrl-D, exit the loop regardless of the window state.
        if (key == "^D") return String(lhs);

        // If the user input is exit key, return an empty vector (indicate cancellation).
        if ((key == "q") or (key == "Q")) return String(lhs);

        // If the user input is enter key, return the selected path(s).
        if ((key == "^M") or (key == "^J"))
            return lhs_rest + shlex_join(model.get_selected());

        // Otherwise, process the key and update the model state.
        const String updated_grep_str = model.process_key(key, window.get_grep_str());
        window.set_grep_str(updated_grep_str);
    }

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
