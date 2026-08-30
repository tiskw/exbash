////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: file_model.hxx                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FILE_MODEL_HXX
#define FILE_MODEL_HXX

// Include custom headers.
#include "config.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

class FileModel
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        FileModel(Path path_ini, const OmniPickerConfig& cfg);
        // Constructor of the FileModel class.
        //
        // [Args]
        //   path_ini (Path)                   : [IN] The initial directory path to start the filer in.
        //   cfg      (const OmniPickerConfig&): [IN] The configuration values for the filer.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Public member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        const Vector<FileItem>& get_contents_filt(void);
        const Vector<FileItem>& get_contents_prev(void);
        // Getter functions for the filtered and previous directory contents.
        //
        // [Returns]
        //   (const Vector<FileItem>&): A reference to the vector of filtered or previous directory contents.

        int32_t get_focus_filt(void) const;
        int32_t get_focus_prev(void) const;
        // Getter functions for the focus indices of the filtered and previous directory contents.
        //
        // [Returns]
        //   (int32_t): The index of the focused item in the filtered or previous directory contents.

        FileItem get_item(void) const;
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

        void update(const Path& path, bool showdot, StringView grep_str);
        // Update the model state based on the current directory, showdot flag, and grep filter string.
        //
        // [Args]
        //   path     (const Path&): [IN] The current directory path.
        //   showdot  (bool)       : [IN] Whether to show hidden (dot) files.
        //   grep_str (StringView) : [IN] The current grep filter string.

        Vector<String> get_preview(const String& path, int32_t max_height = 64);
        // Get the preview lines for a given item in the current directory.
        //
        // [Args]
        //   path       (const String&): [IN] The name of the item to preview.
        //   max_height (int32_t)      : [IN] The maximum number of lines to return in the preview.

    private:

        Path path;
        // Current directory path.

        Vector<FileItem> contents_main;
        Vector<FileItem> contents_filt;
        Vector<FileItem> contents_prev;
        // Lists of items in the current directory, filtered items, and parent directory items.

        int32_t focus_filt;
        // Index of the currently focused item in the filtered contents.

        bool showdot;
        // Whether to show hidden (dot) files.

        String grep_str;
        // Current grep filter string.

        Map<String, Vector<String>> preview_cache;
        // Cache of preview results keyed by resolved path string.
        // NOTE: You can include the registered time of the preview contents in the cache key to avoid
        //       stale previews when the file changes, however, this is not implemented. Because this
        //       filer is a plugin of ExBash and the life time of this filer is assumed to be very short.

        const OmniPickerConfig& cfg;
        // Configuration values.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
