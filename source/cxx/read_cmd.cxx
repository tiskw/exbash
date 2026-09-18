////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: read_cmd.cxx                                                                ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "read_cmd.hxx"

// Include the headers of custom modules.
#include "async_comp.hxx"
#include "char_x.hxx"
#include "config.hxx"
#include "edit_helper.hxx"
#include "history_manager.hxx"
#include "terminal.hxx"
#include "text_editor_emacs.hxx"
#include "text_editor_vi.hxx"
#include "utf8.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    std::unique_ptr<TextEditor> create_editor(StringView editor_name, StringView lhs_ini, StringView rhs_ini, const Deque<String>& hists)
    // Factory function to create a TextEditor instance based on the specified editor name.
    //
    // [Args]
    //   editor_name (StringView)          : [IN] Name of the text editor to create ("emacs" or "vi").
    //   lhs_ini     (StringView)          : [IN] Initial left-hand side string for the text editor.
    //   rhs_ini     (StringView)          : [IN] Initial right-hand side string for the text editor.
    //   hists       (const Deque<String>&): [IN] History list for the text editor.
    //
    // [Returns]
    //   (std::unique_ptr<TextEditor>): A unique pointer to the created TextEditor instance.
    //
    {   // {{{

        switch (hash(editor_name))
        {
            case hash("emacs"): return std::make_unique<TextEditorEmacs>(lhs_ini, rhs_ini, hists);
            case hash("vi")   : return std::make_unique<TextEditorVi>   (lhs_ini, rhs_ini, hists);
            default           : return std::make_unique<TextEditorEmacs>(lhs_ini, rhs_ini, hists);
        }

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Published functions
////////////////////////////////////////////////////////////////////////////////////////////////////

ReadCmdOut readcmd(StringView lhs_ini, StringView rhs_ini, const Deque<String>& hists, StringView editor_name, StringView inputs, const ExBashConfig& cfg)
{   // {{{

    // Set of keys that stops readcmd function.
    Set<StringView> stop_keys;
    for (const auto& pair : cfg.keybinds)
        stop_keys.emplace(pair.first);

    // Get terminal size.
    const Size term_size = get_terminal_size();

    // Instantiate necessary classes.
    HistManager histmn = HistManager(hists);

    // Instantiate a text editor based on the specified editor name.
    UniqPtr<TextEditor> editor = create_editor(editor_name, lhs_ini, rhs_ini, hists);

    // Get prefix and postfix strings for history completion.
    StringView histhint_pre  = StringView(cfg.histhint_pre);
    StringView histhint_post = StringView(cfg.histhint_post);

    // Asynchronous completion is used only for interactive input mode.
    const bool use_async_comp = inputs.empty();

    // Exactly one EditHelper exists in this function:
    //   - use_async_comp == true : opt_async_comp owns it (accessed via complete_sync).
    //   - use_async_comp == false: opt_edit_helper owns it.
    Optional<EditHelper> opt_edit_helper;
    Optional<AsyncComp>  opt_async_comp;
    if (use_async_comp) { opt_async_comp.emplace(cfg.area_height, term_size.cols, cfg);  }
    else                { opt_edit_helper.emplace(cfg.area_height, term_size.cols, cfg); }

    // NOTE: TermUserIF is declared at the end intentionally, bacause the object destruction
    // order is the reverse of the declaration, so the terminal is restored to canonical mode
    // before any completion machinery is destroyed.
    TermUserIF termui = TermUserIF(cfg.area_height, term_size.cols);

    while (true)
    {
        // Get editing buffer.
        // NOTE: that these variables are invalidated after "editor.edit()".
        //       Do not store or use them beyond the "editor.edit()" call.
        const StringView lhs = editor->get_lhs();
        const StringView rhs = editor->get_rhs();

        // Launch asynchronous completion for the current user input.
        if (use_async_comp)
            opt_async_comp->launch_async_completion(lhs);

        // Get completion candidates.
        Vector<String> clines = use_async_comp ? opt_async_comp->get_completion_result()
                                               : opt_edit_helper->candidate(lhs);

        // Get history completion.
        StringView hist_comp = histmn.complete(lhs);

        // Select ps1 buffer.
        const StringView ps1 = StringView((editor->get_mode() == TextEditor::Mode::INSERT) ? cfg.ps1i : cfg.ps1n);
        const StringView ps2 = StringView(cfg.ps2);

        // Re-draw terminal.
        termui.update(lhs, rhs, ps1, ps2, clines, hist_comp, histhint_pre, histhint_post);

        // Get user input.
        CharX cx = (inputs.size() > 0) ? utf8_decode_next_charx(inputs.data()):
                    use_async_comp     ? termui.getch(opt_async_comp->get_wakeup_fd()):
                                         termui.getch(-1);

        // Update the "inputs" view if it's not nullptr.
        inputs = (inputs.size() > 0) ? (StringView(inputs.begin() + cx.size(), inputs.end())) : inputs;

        // Exit if one of the step key is typed.
        if (stop_keys.contains(cx.printable()))
            return ReadCmdOut(lhs, rhs, cx.printable(), inputs);

        // Do nothing if the character is empty.
        if (cx.size() == 0)
            continue;

        // Execute the command if the input is a multi-byte character.
        if (cx.size() > 1)
            editor->edit(cx.view());

        // Otherwise (size is 1), process the input character.
        else
        {
            // Process input character.
            switch (*cx.c_str())
            {
                // Exit function if Ctrl-C is pressed.
                case 0x00:
                case 0x03:
                    return ReadCmdOut("^C", "", "", inputs);

                // Exit function if Ctrl-D is pressed.
                case 0x04:
                    return ReadCmdOut("^D", "", "", inputs);

                // History completion if Ctrl-E is pressed.
                case 0x05:
                    editor->set(String(lhs) + String(histmn.complete(lhs)) + " ", rhs);
                    break;

                // Execute completion if Ctrl-I (= horizontal tab) is pressed.
                case 0x09:
                    if (use_async_comp)
                    {
                        editor->set(opt_async_comp->complete_sync(lhs), rhs);
                    }
                    else
                    {
                        opt_edit_helper->candidate(lhs);
                        editor->set(opt_edit_helper->complete(lhs), rhs);
                    }
                    break;

                // Exit function if ENTER is pressed.
                case '\n':
                case '\r':
                    return ReadCmdOut(lhs, rhs, "", inputs);

                // Otherwise update editing buffer.
                default: editor->edit(cx.view());
            }
        }
    }

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
