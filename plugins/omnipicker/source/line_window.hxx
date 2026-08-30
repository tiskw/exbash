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

class LineWindow
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

         LineWindow(const Colors colors, StringView user_input);
        ~LineWindow(void);
        // Constructor of the LineWindow class.
        //
        // [Args]
        //   user_input (StringView): [IN] Optional string to simulate user input.

        // Destructor of the LineWindow class.

        LineWindow(const LineWindow&) = delete;
        LineWindow& operator = (const LineWindow&) = delete;
        // Prohibit copying and assignment of LineWindow instances.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // 
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

        int32_t height, width;
        // Height of the terminal window (number of rows).

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private drawing utilities
        ////////////////////////////////////////////////////////////////////////////////////////////

        void listbox(Rect rect, const Vector<LineItem>& contents, int32_t focus);
        // Draw a list with the focused item highlighted (A_REVERSE).
        //
        // [Args]
        //   rect     (const Rect&)            : [IN] The rectangle to draw the list in (relative to win).
        //   contents (const Vector<LineItem>&): [IN] The list of items to draw.
        //   focus    (int32_t)                : [IN] The index of the item to highlight with A_REVERSE.

        void update_layout(void);
        // Update the layout of the filer window.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
