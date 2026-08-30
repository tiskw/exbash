////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: grid_picker.hxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GRID_PICKER_HXX
#define GRID_PICKER_HXX

// Include custom headers.
#include "config.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

String run_grid_picker(StringView lhs, const OmniPickerConfig& cfg, StringView user_input = "");
// Run the grid picker TUI and return the selected entries joined by spaces.
//
// [Args]
//   lhs        (StringView)             : [IN] The left-hand side string.
//   cfg        (const OmniPickerConfig&): [IN] The configuration values.
//   user_input (StringView)             : [IN] An optional string of keystrokes.
//
// [Returns]
//   (String): The selected history entries joined by spaces, or empty string if aborted.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
