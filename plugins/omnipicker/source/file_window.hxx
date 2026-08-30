////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: file_window.hxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FILE_WINDOW_HXX
#define FILE_WINDOW_HXX

// Include custom headers.
#include "cui.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Data type declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

struct FileLayout
{
    int32_t h, w1, w2, w3, x1, x2, x3;
    // The layout dimensions and positions of the three panels in the filer window.
    //
    // [Members]
    //   h  (int32_t): Height of the panels (excluding borders).
    //   w1 (int32_t): Width of the left panel (excluding borders).
    //   w2 (int32_t): Width of the middle panel (excluding borders).
    //   w3 (int32_t): Width of the right panel (excluding borders).
    //   x1 (int32_t): X-coordinate of the left panel's top-left corner.
    //   x2 (int32_t): X-coordinate of the middle panel's top-left corner.
    //   x3 (int32_t): X-coordinate of the right panel's top-left corner.
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

class FileWindow
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        FileWindow(float w1_ratio, float w2_ratio, const Colors colors, StringView user_input = "");
        // Constructor of the FileWindow class.
        //
        // [Args]
        //   w1_ratio   (float)       : [IN] Width ratio of the left panel (0.0 to 1.0).
        //   w2_ratio   (float)       : [IN] Width ratio of the middle panel (0.0 to 1.0).
        //   colors     (const Colors): [IN] Color values for the filer window.
        //   user_input (StringView)  : [IN] Optional string to simulate user input.

        ~FileWindow(void);
        // Destructor of the FileWindow class.

        FileWindow(const FileWindow&) = delete;
        FileWindow& operator = (const FileWindow&) = delete;
        // Prohibit copying and assignment of FileWindow instances.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // 
        ////////////////////////////////////////////////////////////////////////////////////////////

        void draw(const Vector<FileItem>& contents_filt, int32_t focus_filt,
                  const Vector<FileItem>& contents_prev, int32_t focus_prev,
                  const Vector<String>& preview_lines);
        // Draw the filer window with the given contents and focus indices.
        //
        // [Args]
        //   contents_filt (const Vector<FileItem>&): [IN] The list of items in the filtered view.
        //   focus_filt    (int32_t)                : [IN] The index of the focused item in the filtered view.
        //   contents_prev (const Vector<FileItem>&): [IN] The list of items in the preview view.
        //   focus_prev    (int32_t)                : [IN] The index of the focused item in the preview view.
        //   preview_lines (const Vector<String>&)  : [IN] The lines of text to display in the preview window.

        String get_grep_str(void) const noexcept;
        void   set_grep_str(StringView grep_str);
        // Get and set the current grep filter string.

        String getkey(void);
        // Process one keystroke (or consume the pre-loaded key string k).
        // Returns the key string if not consumed internally, or "" if consumed.
        //
        // [Returns]
        //   (String): The key string, or empty string (if consumed internally).

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        CursesScreen screen;
        // The CursesScreen object for handling ncurses drawing and input.

        float w1_ratio, w2_ratio;
        // Width ratios for the left and middle panels (0.0 to 1.0).

        bool has_grep_win, has_preview_win;
        // Flags indicating whether the grep window and preview window are currently displayed.

        String grep_str;
        // Current grep filter string.

        int32_t preview_win_pos;
        // Current scroll position of the preview window.

        FileLayout layout;
        // Current layout of the filer window.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private drawing utilities
        ////////////////////////////////////////////////////////////////////////////////////////////

        void listbox(Rect rect, const Vector<FileItem>& contents, int32_t focus);
        // Draw a list with the focused item highlighted (A_REVERSE).
        //
        // [Args]
        //   rect     (const Rect&)            : [IN] The rectangle to draw the list in (relative to win).
        //   contents (const Vector<FileItem>&): [IN] The list of items to draw.
        //   focus    (int32_t)                : [IN] The index of the item to highlight with A_REVERSE.

        void update_layout(void);
        // Update the layout of the filer window.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
