////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: file_preview.hxx                                                            ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FILE_PREVIEW_HXX
#define FILE_PREVIEW_HXX

// Include the headers of STL.
#include <filesystem>
#include <variant>

// Include custom headers.
#include "config.hxx"
#include "dtypes.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<FileItem> listdir(const Path& dirpath, bool showdot, bool extra, int max_items = 100);
// List directory contents as a sorted vector of FileItem (dirs first, then files).
//
// [Args]
//   dirpath   (const Path&): [IN] The directory path to list.
//   showdot   (bool)       : [IN] Whether to include hidden files (those starting with a dot).
//   extra     (bool)       : [IN] Whether to include extra info (permissions string, owner, group, size, mtime) in the right-side string of FileItem.
//   max_items (int)        : [IN] Maximum number of items to list (default: 100).
//
// [Returns]
//   (Vector<FileItem>): A vector of FileItem representing the directory contents.

Vector<String> preview(const Path& path, int height, const OmniPickerConfig& cfg);
// Generate a preview of the given file or directory.
//
// [Args]
//   path   (const Path&)            : [IN] Target path to preview.
//   height (int)                    : [IN] Maximum number of lines/items to return.
//   cfg    (const OmniPickerConfig&): [IN] Configuration values for omnipicker.
//
// [Returns]
//   (Vector<String>): A vector of strings representing the preview lines for the given path.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
