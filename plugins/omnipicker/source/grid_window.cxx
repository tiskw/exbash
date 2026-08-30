////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: grid_window.cxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "grid_window.hxx"

// Include custom headers.
#include "strextra.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

GridWindow::GridWindow(const Colors colors, StringView user_input)
    : screen(colors, user_input), has_grep_win(false), grep_str(""), num_rows(0), num_cols(0)
{ }

GridWindow::~GridWindow(void)
{ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// Member function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

void GridWindow::draw(const Vector<LineItem>& items_filt, int32_t focus_filt)
{   // {{{

    // Get the terminal dimensions.
    const auto [h, w] = this->screen.update_term_size();

    // Define the positions of the three panels and the other windows.
    Rect pos_grep_win = Rect{4, h / 2, w - 8, 3};

    // Clear the entire screen before drawing the new contents.
    this->screen.erase();

    // Draw the header and the grid of items.
    draw_header(items_filt, focus_filt, w);

    // Draw the grid of items.
    draw_grid(items_filt, focus_filt, h, w);

    // If the grep window is open, draw it.
    if (this->has_grep_win)
    {
        // Draw a box around the grep window.
        this->screen.rectbox(pos_grep_win.y, pos_grep_win.x, pos_grep_win.h, pos_grep_win.w);

        // Draw the current grep filter string inside the grep window.
        this->screen.addstr_a(pos_grep_win.y + 1, pos_grep_win.x + 1, this->grep_str);
    }

    // Update the screen.
    this->screen.refresh();

}   // }}}

String GridWindow::getkey(void)
{   // {{{

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

String GridWindow::get_grep_str(void) const noexcept
{   // {{{

    return this->grep_str;

}   // }}}

void GridWindow::set_grep_str(StringView grep_str)
{   // {{{

    this->grep_str = String(grep_str);

}   // }}}

void GridWindow::draw_header(const Vector<LineItem>& items, int32_t selected, int32_t w)
{   // {{{

    // Create the header string.
    String header = items.empty()
        ? "Selected token: (none)"
        : std::format("Selected token:  [{}/{}: {}]", selected + 1, items.size(), items[selected].line);

    // Draw the header string at the top of the window.
    this->screen.addstr_a(0, 0, clipstr(header, w));

    // Draw the instruction string below the header.
    this->screen.addstr_a(1, 0, clipstr("Move: arrow keys or hjkl    Select: SPACE    Cancel: q", w));

};  // }}}

void GridWindow::draw_grid(const Vector<LineItem>& items, int32_t selected, int32_t h, int32_t w)
{   // {{{

    // Constants for the grid layout.
    this->num_cols = 6;
    constexpr int32_t grid_top  = 2;
    constexpr int32_t row_pitch = 2;

    // Compute layout info.
    const int32_t col_width = w / this->num_cols;
    this->num_rows  = (h - grid_top - 1) / row_pitch;

    // Draw grid frame.
    for (int32_t row = 0; row <= num_rows; ++row)
    {
        // Y position of the current row in the grid.
        const int32_t y = grid_top + row * row_pitch;

        // Draw the horizontal line for the current row.
        if      (row == 0      ) this->draw_grid_hline(y, this->num_cols, col_width, ACS_ULCORNER, ACS_TTEE, ACS_URCORNER);
        else if (row < num_rows) this->draw_grid_hline(y, this->num_cols, col_width, ACS_LTEE,     ACS_PLUS, ACS_RTEE    );
        else                     this->draw_grid_hline(y, this->num_cols, col_width, ACS_LLCORNER, ACS_BTEE, ACS_LRCORNER);

        // Draw the vertical lines for the current row.
        if (row < num_rows)
            this->draw_grid_vline(y + 1, this->num_cols, col_width, row_pitch);
    }

    // Compute the index of the first item to display on the current page.
    const int32_t idx_page  = selected / (this->num_cols * num_rows);
    const int32_t idx_begin = idx_page * (this->num_cols * num_rows);

    for (int32_t idx = 0; idx < (this->num_cols * num_rows); ++idx)
    {
        // Compute the index of the current item in the items vector.
        const int32_t idx_item = idx_begin + idx;
        if (static_cast<SizeType>(idx_item) >= items.size())
            break;

        // Compute the row and column of the current item in the grid.
        const int32_t row = idx / this->num_cols;
        const int32_t col = idx % this->num_cols;
        const int32_t y   = grid_top + row * row_pitch + 1;
        const int32_t x   = col * col_width + 1;

        // Get the current item from the items vector.
        const LineItem& item = items[static_cast<size_t>(idx_item)];

        // Create the string to display for the current item.
        String str = fitstr((item.star ? "* " : "  ") + item.line, col_width - 2);

        // Set the line attributes.
        int32_t line_attr = A_NORMAL;
        if (idx_item == selected) line_attr |= A_REVERSE;
        if (item.star           ) line_attr |= COLOR_PAIR(3);

        // Draw the string for the current item.
        this->screen.addstr_a(y, x, str, line_attr);
    }

}   // }}}

void GridWindow::draw_grid_hline(int32_t y, int32_t num_cols, int32_t col_width, chtype ch1, chtype ch2, chtype ch3)
{   // {{{

    for (int32_t col = 0; col < num_cols; ++col)
    {
        // Compute the x position.
        const int32_t x = col * col_width;

        // Draw the corner character at the start of the line segment.
        if (col == 0) { this->screen.addchr(y, x, ch1); }
        else          { this->screen.addchr(y, x, ch2); }

        // Draw the horizontal line segment.
        this->screen.line_h(y, x + 1, col_width - 1);
    }

    // Draw the last corner character at the end of the line.
    this->screen.addchr(y, num_cols * col_width, ch3);

}   // }}}

void GridWindow::draw_grid_vline(int32_t y, int32_t num_cols, int32_t col_width, int32_t row_pitch)
{   // {{{

    // Draw the vertical line segment.
    for (int32_t col = 0; col <= num_cols; ++col)
        this->screen.line_v(y, col * col_width, row_pitch - 1);

}   // }}}

Tuple<int32_t, int32_t> GridWindow::get_grid_shape(void)
{   // {{{

    return {this->num_rows, this->num_cols};

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
