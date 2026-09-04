////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: file_picker.hxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FILE_PICKER_HXX
#define FILE_PICKER_HXX

// Include custom headers.
#include "config.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

String run_file_picker(StringView lhs, const OmniPickerConfig& cfg, StringView user_input);
// Initialise ncurses, run the file picker, tear down ncurses, and return the updated left-hand side
// string where the selected file path(s) are appended with shell-escaped format.
//
// [Args]
//   lhs        (StringView)             : [IN] The left-hand side string.
//   cfg        (const OmniPickerConfig&): [IN] The configuration values.
//   user_input (StringView)             : [IN] An optional string of keystrokes.
//
// [Returns]
//   (String): The updated left-hand side string.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
