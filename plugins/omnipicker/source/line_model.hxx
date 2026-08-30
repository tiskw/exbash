////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: line_model.hxx                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LINE_MODEL_HXX
#define LINE_MODEL_HXX

// Include custom headers.
#include "config.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

class LineModel
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        explicit LineModel(const Vector<String>& items);
        // Constructor of the LineModel class.
        //
        // [Args]
        //   items (const Vector<String>&): [IN] The initial list of items to display in the filer.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Public member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        const Vector<LineItem>& get_contents_filt(void);
        // Getter functions for the filtered and previous directory contents.
        //
        // [Returns]
        //   (const Vector<LineItem>&): A reference to the vector of filtered or previous directory contents.

        int32_t get_focus_filt(void) const;
        // Getter functions for the focus indices of the filtered and previous directory contents.
        //
        // [Returns]
        //   (int32_t): The index of the focused item in the filtered or previous directory contents.

        LineItem get_item(void) const;
        // Get the currently focused item in the filtered contents.

        Vector<String> get_selected(void) const;
        // Returns the selected items as a vector of strings.
        // If no items are selected, returns the currently focused item.
        //
        // [Returns]
        //   (Vector<String>): A vector of selected item names.

        String process_key(StringView key, StringView grep_str_win);
        // Process a key input and update the model state accordingly.
        //
        // [Args]
        //   key          (StringView): [IN] The key input to process.
        //   grep_str_win (StringView): [IN] The current grep filter string from the window.
        //
        // [Returns]
        //   (String): The grep filter string after processing the key input.

        void update(StringView grep_str);
        // Update the model state based on the current directory, showdot flag, and grep filter string.
        //
        // [Args]
        //   grep_str (StringView): [IN] The current grep filter string.

    protected:

        Vector<LineItem> contents_main;
        Vector<LineItem> contents_filt;
        // Lists of items in the main and filtered contents.

        int32_t focus_filt;
        // Index of the currently focused item in the filtered contents.

        String grep_str;
        // Current grep filter string.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
