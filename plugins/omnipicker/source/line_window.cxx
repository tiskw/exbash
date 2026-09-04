////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: window.cxx                                                                  ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "line_window.hxx"

// Include custom headers.
#include "strextra.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

LineWindow::LineWindow(const Colors colors, StringView user_input) : screen(colors, user_input), has_grep_win(false)
{   // {{{

    // Update the layout of the filer window.
    this->update_layout();

}   // }}}

LineWindow::~LineWindow(void)
{   // {{{

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Member function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

void LineWindow::draw(const Vector<LineItem>& contents_filt, int32_t focus_filt)
{   // {{{

    // Update the layout of the filer window.
    this->update_layout();

    // Assign short names to the layout dimensions (just for readability).
    const int32_t h = this->height;
    const int32_t w = this->width;

    // Define the positions of the three panels and the other windows.
    Rect pos_list_filt = Rect{1, 1, w, h};
    Rect pos_grep_win  = Rect{4, h / 2, w - 8, 3};

    // Clear the entire screen before drawing the new contents.
    this->screen.erase();

    // Draw the three panels of the filer window.
    this->screen.rectbox(0, 0, h + 2, w + 2);
    this->listbox(pos_list_filt, contents_filt, focus_filt);

    // If the grep window is open, draw it.
    if (this->has_grep_win)
    {
        // Draw a box around the grep window.
        this->screen.rectbox(pos_grep_win.y, pos_grep_win.x, pos_grep_win.h, pos_grep_win.w);

        // Draw the current grep filter string inside the grep window.
        this->screen.addstr_a(pos_grep_win.y + 1, pos_grep_win.x + 1, this->grep_str);
    }

    this->screen.refresh();

}   // }}}

String LineWindow::getkey(void)
{   // {{{

    // Update the layout of the filer window before processing the key press.
    this->update_layout();

    // Get a key press from the user (or from the pre-loaded user_input string).
    const String key = this->screen.getkey();

    //----------------------------------------------------------------------------------------------
    // Case 1: The key press on the grep window
    //----------------------------------------------------------------------------------------------
    if (this->has_grep_win)
    {
        switch (hash(key))
        {
            // Confirm the grep filter.
            case hash("^J"):
            case hash("^M"):
                this->has_grep_win = false;
                return "";

            // Remove the last character from the grep filter string.
            case hash("^H"):
            case hash("KEY_BACKSPACE"):
                this->grep_str = remove_last_utf8_char(this->grep_str);
                return "";

            default:
                if (not key.starts_with("KEY_"))
                    this->grep_str += key;
                return "";
        }
    }

    //----------------------------------------------------------------------------------------------
    // Case 2: The key press on the list window
    //----------------------------------------------------------------------------------------------
    else
    {
        switch (hash(key))
        {
           // Open the grep filter.
            case hash("/"):
                this->has_grep_win = true;
                return "";

            // Not consumed
            default:
                return key;
        }
    }

    return key;

}   // }}}

String LineWindow::get_grep_str(void) const noexcept
{   // {{{

    return this->grep_str;

}   // }}}

void LineWindow::set_grep_str(StringView grep_str)
{   // {{{

    this->grep_str = String(grep_str);

}   // }}}

void LineWindow::listbox(Rect rect, const Vector<LineItem>& contents, int32_t focus)
{   // {{{

    // Do nothing if the contents are empty.
    if (contents.empty()) return;

    // Clip the focus index to ensure it is within the valid range of the contents vector.
    focus = clip(focus, 0, static_cast<int32_t>(contents.size()) - 1);

    const int32_t n  = static_cast<int32_t>(contents.size());
    const int32_t c0 = rect.h / 2;

    // Determine which slice of the list is visible.
    const int32_t index_bgn = clip(focus - c0, 0, std::max(0, n - rect.h));
    const int32_t index_end = std::min(n, index_bgn + rect.h);
    const int32_t adj_focus = focus - index_bgn;

    for (int32_t row = 0; row < index_end - index_bgn; ++row)
    {
        const LineItem& item  = contents[static_cast<size_t>(index_bgn + row)];
        const int32_t   avail = rect.w - 5;

        String left_clip = clipstr(item.line, std::max(0, avail));
        const int32_t pad = std::max(0, avail - strwidth(left_clip));
        String line = "  " + left_clip + String(pad, ' ');

        // Highlight the focused line with A_REVERSE, and use A_NORMAL for other lines.
        int32_t line_attr = (row == adj_focus) ? A_REVERSE : A_NORMAL;

        if (item.star) line = "* " + line.substr(2);

        this->screen.addstr_a(rect.y + row, rect.x, line, line_attr);
    }

}   // }}}

void LineWindow::update_layout(void)
{   // {{{

    // Get the terminal dimensions.
    const auto [term_h, term_w] = this->screen.update_term_size();

    // Update the height of the window.
    this->height = term_h - 2;
    this->width  = term_w - 2;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
