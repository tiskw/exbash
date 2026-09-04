////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: window.hxx                                                                  ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WINDOW_HXX
#define WINDOW_HXX

// Include custom headers.
#include "cui.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Data type declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

class GridWindow
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

         GridWindow(const Colors colors, StringView user_input);
        ~GridWindow(void);
        // Constructor and destructor of the GridWindow class.
        //
        // [Args]
        //   colors     (const Colors): [IN] The color configuration for the window.
        //   user_input (StringView)  : [IN] Optional string to simulate user input.

        GridWindow(const GridWindow&) = delete;
        GridWindow& operator = (const GridWindow&) = delete;
        // Prohibit copying and assignment of GridWindow instances.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Public member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void draw(const Vector<LineItem>& contents_filt, int32_t focus_filt);
        // Draw the filer window with the given contents and focus indices.
        //
        // [Args]
        //   contents_filt (const Vector<LineItem>&): [IN] The list of items in the filtered view.
        //   focus_filt    (int32_t)                : [IN] The index of the focused item in the filtered view.

        String get_grep_str(void) const noexcept;
        void   set_grep_str(StringView grep_str);
        // Get and set the current grep filter string.

        String getkey(void);
        // Process one keystroke (or consume the pre-loaded key string k).
        // Returns the key string if not consumed internally, or "" if consumed.
        //
        // [Returns]
        //   (String): The key string if not consumed internally (e.g. for quitting or confirming selection), or an empty string if the key was consumed for internal state changes (e.g. navigation, toggling preview, opening grep window).

        Tuple<int32_t, int32_t> get_grid_shape(void);
        // Get the number of rows and columns in the grid.
        //
        // [Returns]
        //   (int32_t): The number of rows.
        //   (int32_t): The number of columns.

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        CursesScreen screen;
        // The CursesScreen object for handling ncurses drawing and input.

        bool has_grep_win;
        // Flags indicating whether the grep window and preview window are currently displayed.

        String grep_str;
        // Current grep filter string.

        int32_t num_rows, num_cols;
        // Height of the terminal window (number of rows).

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private drawing utilities
        ////////////////////////////////////////////////////////////////////////////////////////////

        void draw_header(const Vector<LineItem>& items, int32_t selected, int32_t w);
        // Draw the header section of the window.
        //
        // [Args]
        //   items    (const Vector<LineItem>&): [IN] The list of items to display in the header.
        //   selected (int32_t)                : [IN] The index of the currently selected item.
        //   w        (int32_t)                : [IN] The width of the window.

        void draw_grid(const Vector<LineItem>& items, int32_t selected, int32_t h, int32_t w);
        // Draw the grid of items in the window.
        //
        // [Args]
        //   items    (const Vector<LineItem>&): [IN] The list of items to display in the grid.
        //   selected (int32_t)                : [IN] The index of the currently selected item.
        //   h        (int32_t)                : [IN] The height of the window.
        //   w        (int32_t)                : [IN] The width of the window.

        void draw_grid_hline(int32_t y, int32_t num_cols, int32_t col_width, chtype ch1, chtype ch2, chtype ch3);
        // Draw a horizontal line in the grid with specified characters for corners and intersections.
        //
        // [Args]
        //   y        (int32_t) : [IN] The Y coordinate to draw the horizontal line.
        //   num_cols (int32_t) : [IN] The number of columns in the grid.
        //   col_width(int32_t) : [IN] The width of each column in the grid.
        //   ch1      (chtype)  : [IN] The character to use for the left corner of the line.
        //   ch2      (chtype)  : [IN] The character to use for the intersections between columns.
        //   ch3      (chtype)  : [IN] The character to use for the right corner of the line.

        void draw_grid_vline(int32_t y, int32_t num_cols, int32_t col_width, int32_t row_pitch);
        // Draw vertical lines in the grid to separate columns.
        //
        // [Args]
        //   y         (int32_t) : [IN] The Y coordinate to start drawing the vertical lines.
        //   num_cols  (int32_t) : [IN] The number of columns in the grid.
        //   col_width (int32_t) : [IN] The width of each column in the grid.
        //   row_pitch (int32_t) : [IN] The vertical spacing between rows in the grid.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
