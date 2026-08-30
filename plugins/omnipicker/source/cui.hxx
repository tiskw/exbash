////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: cui.hxx                                                                     ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CUI_HXX
#define CUI_HXX

// Include custom headers.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

class CursesScreen
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

         CursesScreen(const Colors colors, StringView user_input);
        ~CursesScreen(void);
        // Constructor and descructor of CursesScreen class.
        //
        // [Args]
        //   colors     (const Colors): [IN] The color values to use for the TUI window.
        //   user_input (StringView)  : [IN] An optional string to simulate as user input.

        CursesScreen(const CursesScreen&) = delete;
        CursesScreen& operator = (const CursesScreen&) = delete;
        // Prohibit copying and assignment of CursesScreen instances.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Public member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void addchr(int32_t y, int32_t x, const chtype ch);
        // Draw a character with the specified attribute.
        //
        // [Args]
        //   y    (int32_t)     : [IN] The Y coordinate to start drawing (relative to win).
        //   x    (int32_t)     : [IN] The X coordinate to start drawing (relative to win).
        //   ch   (const chtype): [IN] The character to draw.
        //   attr (int32_t)     : [IN] The attribute to apply to the text (e.g., A_BOLD, A_REVERSE, COLOR_PAIR(n)).

        void addstr_a(int32_t y, int32_t x, const String& text, int32_t attr = A_NORMAL);
        // Draw a string with the specified attribute.
        //
        // [Args]
        //   y     (int32_t)      : [IN] The Y coordinate to start drawing (relative to win).
        //   x     (int32_t)      : [IN] The X coordinate to start drawing (relative to win).
        //   text  (const String&): [IN] The text to draw.
        //   attr  (int32_t)      : [IN] The attribute to apply to the text (e.g., A_BOLD, A_REVERSE, COLOR_PAIR(n)).

        void addstr_c(int32_t y, int32_t x, const String& text, int32_t max_w);
        // Draw a string that may contain ANSI SGR escape sequences.
        //
        // [Args]
        //   y     (int32_t)      : [IN] The Y coordinate to start drawing (relative to win).
        //   x     (int32_t)      : [IN] The X coordinate to start drawing (relative to win).
        //   text  (const String&): [IN] The text to draw, which may contain ANSI SGR escape sequences for coloring and styling.
        //   max_w (int)          : [IN] The maximum width to draw (used for clipping the string to fit in the available space).
        //
        // [Returns]
        //   (void): This function does not return a value.

        void erase(void) noexcept;
        // Clear the entire screen.

        String getkey(void);
        // Get a key press from the terminal or the pre-loaded user_input string.
        //
        // [Returns]
        //   (String): The key string representing the keystroke.

        void line_h(int32_t y, int32_t x, int32_t w);
        // Draw a horizontal line with the specified attribute.
        //
        // [Args]
        //   y    (int32_t) : [IN] The Y coordinate to start drawing (relative to win).
        //   x    (int32_t) : [IN] The X coordinate to start drawing (relative to win).
        //   w    (int32_t) : [IN] The width of the line to draw.
        //   attr (int32_t) : [IN] The attribute to apply to the line (e.g., A_BOLD, A_REVERSE, COLOR_PAIR(n)).

        void line_v(int32_t y, int32_t x, int32_t h);
        // Draw a vertical line with the specified attribute.
        //
        // [Args]
        //   y    (int32_t) : [IN] The Y coordinate to start drawing (relative to win).
        //   x    (int32_t) : [IN] The X coordinate to start drawing (relative to win).
        //   h    (int32_t) : [IN] The height of the line to draw.
        //   attr (int32_t) : [IN] The attribute to apply to the line (e.g., A_BOLD, A_REVERSE, COLOR_PAIR(n)).

        void rectbox(int32_t y, int32_t x, int32_t h, int32_t w);
        // Draw a box border around the specified rectangle.
        //
        // [Args]
        //   y (int32_t): [IN] The Y coordinate of the top-left corner of the rectangle (relative to win).
        //   x (int32_t): [IN] The X coordinate of the top-left corner of the rectangle (relative to win).
        //   h (int32_t): [IN] The height of the rectangle.
        //   w (int32_t): [IN] The width of the rectangle.

        void rectbox_with_vlines(const Vector<int32_t>& cols_vline);
        // Draw a box border and optional vertical divider lines.
        //
        // [Args]
        //   cols_vline (const Vector<int32_t>&): [IN] A vector of X coordinates (relative to win).

        void refresh(void) noexcept;
        // Refresh the screen to show the latest changes.

        Tuple<int32_t, int32_t> update_term_size(void) noexcept;
        // Get the current terminal dimensions (height, width).
        //
        // [Returns]
        //   (int32_t, int32_t): A tuple containing the terminal height and width.

    private:

        int32_t h, w;
        // Terminal height and width.

        Window stdscr;
        // The main screen window.

        String user_input_body;
        // Pre-loaded string of keystrokes to simulate as user input.

        StringView user_input_view;
        // View of the remaining unconsumed portion of the user_input string.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
