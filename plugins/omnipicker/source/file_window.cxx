////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: file_window.cxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "file_window.hxx"

// Include custom headers.
#include "strextra.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

FileWindow::FileWindow(float w1_ratio, float w2_ratio, const Colors colors, StringView user_input)
    : screen(colors, user_input), w1_ratio(w1_ratio), w2_ratio(w2_ratio), has_grep_win(false), has_preview_win(false), preview_win_pos(0)
{   // {{{

    // Update the layout of the filer window.
    this->update_layout();

}   // }}}

FileWindow::~FileWindow(void)
{   // {{{

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Member function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

void FileWindow::draw(const Vector<FileItem>& contents_filt, int32_t focus_filt,
                       const Vector<FileItem>& contents_prev, int32_t focus_prev,
                       const Vector<String>& preview_lines)
{   // {{{

    // Update the layout of the filer window.
    this->update_layout();

    // Assign short names to the layout dimensions (just for readability).
    const int32_t h  = this->layout.h;
    const int32_t w1 = this->layout.w1;
    const int32_t w2 = this->layout.w2;
    const int32_t w3 = this->layout.w3;
    const int32_t x1 = this->layout.x1;
    const int32_t x2 = this->layout.x2;
    const int32_t x3 = this->layout.x3;

    // Define the positions of the three panels and the other windows.
    Rect pos_list_prev = Rect{x1, 1, w1, h};
    Rect pos_list_filt = Rect{x2, 1, w2, h};
    Rect pos_prev_area = Rect{x3, 1, w3, h};
    Rect pos_grep_win  = Rect{5, h / 2, w1 + w2 + w3 - 8, 3};
    Rect pos_prev_win  = Rect{4, 4, w1 + w2 + w3 - 4, h - 4};

    // Clear the entire screen before drawing the new contents.
    this->screen.erase();

    // Draw the three panels of the filer window.
    this->screen.rectbox_with_vlines({w1 + 1, w1 + w2 + 2});
    this->listbox(pos_list_prev, contents_prev, focus_prev);
    this->listbox(pos_list_filt, contents_filt, focus_filt);

    // Draw the preview area on the right panel if it is open and there are lines to preview.
    if (not preview_lines.empty())
    {
        // Draw a box around the preview area.
        const int32_t row_max = std::min(h - 2, static_cast<int32_t>(preview_lines.size()));
        for (int32_t row = 0; row < row_max; ++row)
        {
            const String& line = preview_lines[static_cast<size_t>(row)];
            this->screen.addstr_c(pos_prev_area.y + row, pos_prev_area.x + 1, line, pos_prev_area.w - 2);
        }
    }

    // If the grep window is open, draw it.
    if (this->has_grep_win)
    {
        // Draw a box around the grep window.
        this->screen.rectbox(pos_grep_win.y, pos_grep_win.x, pos_grep_win.h, pos_grep_win.w);

        // Draw the current grep filter string inside the grep window.
        this->screen.addstr_a(pos_grep_win.y + 1, pos_grep_win.x + 1, this->grep_str);
    }

    // Draw the preview area on the right panel if it is open and there are lines to preview.
    if (this->has_preview_win and not preview_lines.empty())
    {
        // Clip the cursor position of the preview window.
        this->preview_win_pos = clip(this->preview_win_pos, 0, static_cast<int32_t>(preview_lines.size()) - pos_prev_win.h + 2);

        // Draw a box around the preview window.
        this->screen.rectbox(pos_prev_win.y, pos_prev_win.x, pos_prev_win.h, pos_prev_win.w);

        // Draw the lines of the preview window, starting from the current preview position.
        for (int32_t row = 0; row < (pos_prev_win.h - 2); ++row)
        {
            // Skip if the current row exceeds the number of preview lines.
            if ((this->preview_win_pos + row) >= static_cast<int32_t>(preview_lines.size())) break;

            // Get the line to display.
            const String& line = preview_lines[static_cast<size_t>(this->preview_win_pos + row)];

            // Draw the line inside the preview window, clipped to the available width.
            this->screen.addstr_c(pos_prev_win.y + 1 + row, pos_prev_win.x + 1, line, pos_prev_win.w - 2);
        }
    }

    this->screen.refresh();

}   // }}}

String FileWindow::getkey(void)
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
    // Case 2: The key press on the preview window
    //----------------------------------------------------------------------------------------------
    if (this->has_preview_win)
    {
        switch (hash(key))
        {
            // Close the preview window.
            case hash("q"):
            case hash("^P"):
                this->has_preview_win = false;
                return "";

            // Scroll up in the preview window.
            case hash("k"):
            case hash("KEY_UP"):
                this->preview_win_pos = std::max(0, this->preview_win_pos - 1);
                return "";

            // Scroll down in the preview window.
            case hash("j"):
            case hash("KEY_DOWN"):
                this->preview_win_pos = std::max(0, this->preview_win_pos + 1);
                return "";

            default:
                return "";
        }
    }

    //----------------------------------------------------------------------------------------------
    // Case 3: The key press on the filer window
    //----------------------------------------------------------------------------------------------
    else
    {
        switch (hash(key))
        {
           // Open the grep filter.
            case hash("/"):
                this->has_grep_win = true;
                return "";

            // Open the preview window.
            case hash("^P"):
                this->has_preview_win = true;
                this->preview_win_pos = 0;
                return "";

            // Not consumed
            default:
                return key;
        }
    }

    return key;

}   // }}}

String FileWindow::get_grep_str(void) const noexcept
{   // {{{

    return this->grep_str;

}   // }}}

void FileWindow::set_grep_str(StringView grep_str)
{   // {{{

    this->grep_str = String(grep_str);

}   // }}}

void FileWindow::listbox(Rect rect, const Vector<FileItem>& contents, int32_t focus)
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
        const FileItem& item    = contents[static_cast<size_t>(index_bgn + row)];
        const int32_t   right_w = strwidth(item.right);
        const int32_t   avail   = rect.w - right_w - 5;

        String left_clip = clipstr(item.left, std::max(0, avail));
        const int32_t pad = std::max(0, avail - strwidth(left_clip));
        String line = "  " + left_clip + String(pad, ' ') + "  " + item.right + " ";

        int32_t line_attr = item.attr;
        if (row == adj_focus) line_attr |= A_REVERSE;

        if (item.star) line = "* " + line.substr(2);

        this->screen.addstr_a(rect.y + row, rect.x, line, line_attr);
    }

}   // }}}

void FileWindow::update_layout(void)
{   // {{{

    // Safe-guard the width ratios to ensure they are within reasonable bounds.
    this->w1_ratio = clip(this->w1_ratio, 0.05f, 0.80f);
    this->w2_ratio = clip(this->w2_ratio, 0.05f, 0.90f);
    if (this->w1_ratio + this->w2_ratio > 0.90f)
    {
        this->w1_ratio = 0.20f;
        this->w2_ratio = 0.45f;
    }

    // Get the terminal dimensions.
    const auto [term_h, term_w] = this->screen.update_term_size();

    // Compute the layout of the filer window.
    const int32_t h  = term_h - 2;
    const int32_t w1 = static_cast<int32_t>(term_w * this->w1_ratio) - 1;
    const int32_t w2 = static_cast<int32_t>(term_w * this->w2_ratio) - 1;
    const int32_t w3 = term_w - w1 - w2 - 4;
    const int32_t x1 = 1;
    const int32_t x2 = w1 + 2;
    const int32_t x3 = w1 + w2 + 3;

    // Update the member variable with the new layout.
    this->layout = {h, w1, w2, w3, x1, x2, x3};

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
