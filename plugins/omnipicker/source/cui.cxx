////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: cui.cxx                                                                     ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "cui.hxx"

// Include ncurses.
#include <ncurses.h>

// Include custom headers.
#include "strextra.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// File-local helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Convert a color value from the range [0, 255] to the range [0, 1000] for ncurses.
#define RGB_TO_CURSES(val) (((val) * 1000 + 127) / 255)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

CursesScreen::CursesScreen(const Colors colors, StringView user_input)
{   // {{{

    // Initialize ncurses and set up the terminal state for the TUI.
    this->stdscr = initscr();
    cbreak();
    noecho();
    nonl();

    // Set up the terminal state for the TUI.
    curs_set(0);
    keypad(this->stdscr, TRUE);

    // Setup color pairs.
    start_color();
    use_default_colors();

    // Define the standard color constants for ncurses.
    static const int32_t COLORS[] = {
        COLOR_BLACK, COLOR_RED,     COLOR_GREEN, COLOR_YELLOW,
        COLOR_BLUE,  COLOR_MAGENTA, COLOR_CYAN,  COLOR_WHITE,
    };

    // Initialize the colors if the terminal supports it and can change colors.
    if (has_colors() and can_change_color())
        for (int32_t i = 0; i < 8; ++i)
            init_color(COLORS[i], RGB_TO_CURSES(colors[i][0]), RGB_TO_CURSES(colors[i][1]), RGB_TO_CURSES(colors[i][2]));

    // Initialize color pairs 1–7 (pair 0 is reserved and cannot be used).
    for (int32_t i = 1; i < 8; ++i)
        init_pair(i, COLORS[i], -1);

    // Get the terminal dimensions.
    getmaxyx(stdscr, this->h, this->w);

    // Store the user input string for simulating key presses.
    this->user_input_body = String(user_input);
    this->user_input_view = this->user_input_body;

}   // }}}

CursesScreen::~CursesScreen(void)
{   // {{{

    endwin();

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void CursesScreen::addchr(int32_t y, int32_t x, const chtype ch)
{   // {{{

    mvwaddch(this->stdscr, y, x, ch);

}   // }}}

void CursesScreen::addstr_a(int32_t y, int32_t x, const String& text, int32_t attr)
{   // {{{

    if (attr != A_NORMAL)
        wattron(this->stdscr, attr);

    mvwaddstr(this->stdscr, y, x, text.c_str());

    if (attr != A_NORMAL)
        wattrset(this->stdscr, A_NORMAL);

}   // }}}

void CursesScreen::addstr_c(int32_t y, int32_t x, const String& text, int32_t max_w)
{   // {{{

    // Initialize the current X position.
    int32_t x_cur = x;

    // Initialize the current attribute.
    int32_t attr = A_NORMAL;

    for (const String& token : split_ansi(text))
    {
        // Skip an empty token.
        if (token.empty()) continue;

        // If ANSI escape code, parse it and update the current attribute.
        if (is_ansi(token))
        {
            for (const int32_t color_code : parse_ansi(token))
                attr = ((30 <= color_code) and (color_code <= 37)) ? COLOR_PAIR(color_code - 30) : A_NORMAL;
        }
        else
        {
            // Set the current attribute for the terminal.
            wattrset(this->stdscr, attr);

            // Write the clipped token to the screen with the current attribute.
            const String token_clipped = clipstr(token, max_w - (x_cur - x));
            mvwaddstr(this->stdscr, y, x_cur, token_clipped.c_str());

            // Update the current X position and exit the loop if reached the maximum width.
            x_cur += strwidth(token_clipped);
            if (x_cur >= x + max_w)
                break;
        }
    }

    // Reset the attribute to normal after writing the string.
    wattrset(this->stdscr, A_NORMAL);

}   // }}}

void CursesScreen::erase(void) noexcept
{   // {{{

    werase(this->stdscr);

}   // }}}

String CursesScreen::getkey(void)
{   // {{{

    // If user_input is empty, get a key from the terminal.
    if (this->user_input_view.empty())
    {
        const char* name = keyname(getch());
        return (name != nullptr) ? String(name) : "";
    }

    // Otherwise, return the next key from user_input and update the view.
    const String key = String((user_input_view[0] == '^') ? user_input_view.substr(0, 2) : user_input_view.substr(0, 1));
    this->user_input_view = this->user_input_view.substr(key.size());
    return key;

}   // }}}

void CursesScreen::line_h(int32_t y, int32_t x, int32_t w)
{   // {{{

    mvwhline(this->stdscr, y, x, ACS_HLINE, w);

}   // }}}

void CursesScreen::line_v(int32_t y, int32_t x, int32_t h)
{   // {{{

    mvwvline(this->stdscr, y, x, ACS_VLINE, h);

}   // }}}

void CursesScreen::rectbox(int32_t y, int32_t x, int32_t h, int32_t w)
{   // {{{

    // Draw a rectangular box around the specified rectangle.
    mvwaddch(this->stdscr, y,         x,         ACS_ULCORNER);
    mvwaddch(this->stdscr, y,         x + w - 1, ACS_URCORNER);
    mvwaddch(this->stdscr, y + h - 1, x,         ACS_LLCORNER);
    mvwaddch(this->stdscr, y + h - 1, x + w - 1, ACS_LRCORNER);
    mvwhline(this->stdscr, y,         x + 1,     ACS_HLINE, w - 2);
    mvwhline(this->stdscr, y + h - 1, x + 1,     ACS_HLINE, w - 2);
    mvwvline(this->stdscr, y + 1,     x,         ACS_VLINE, h - 2);
    mvwvline(this->stdscr, y + 1,     x + w - 1, ACS_VLINE, h - 2);

    // Clear the inside of the box.
    for (int32_t y_delta = 1; y_delta < (h - 1); ++y_delta)
        mvwhline(this->stdscr, y + y_delta, x + 1, ' ', w - 2);

}   // }}}

void CursesScreen::rectbox_with_vlines(const Vector<int32_t>& cols_vline)
{   // {{{

    // Draw a rectangular box around the entire window.
    box(this->stdscr, 0, 0);

    // Draw vertical divider lines.
    for (const int32_t col : cols_vline)
    {
        mvwaddch(this->stdscr, 0,     col, ACS_TTEE);
        mvwaddch(this->stdscr, h - 1, col, ACS_BTEE);
        mvwvline(this->stdscr, 1,     col, ACS_VLINE, h - 2);
    }

}   // }}}

void CursesScreen::refresh(void) noexcept
{   // {{{

    wrefresh(this->stdscr);

}   // }}}

Tuple<int32_t, int32_t> CursesScreen::update_term_size(void) noexcept
{   // {{{

    getmaxyx(this->stdscr, this->h, this->w);
    return {this->h, this->w};

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
