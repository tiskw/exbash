////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: line_picker.hxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LINE_PICKER_HXX
#define LINE_PICKER_HXX

// Include custom headers.
#include "config.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

String run_env_picker(StringView lhs, const OmniPickerConfig& cfg, StringView user_input = "");
// Run the environment variable picker TUI and return the selected entries joined by spaces.
//
// [Args]
//   lhs        (StringView)             : [IN] The left-hand side string.
//   cfg        (const OmniPickerConfig&): [IN] The configuration values.
//   user_input (StringView)             : [IN] An optional string of keystrokes.
//
// [Returns]
//   (String): The selected history entries joined by spaces, or empty string if aborted.

String run_hst_picker(StringView lhs, const OmniPickerConfig& cfg, StringView user_input = "");
// Run the history picker TUI and return the selected entries joined by spaces.
//
// [Args]
//   lhs        (StringView)             : [IN] The left-hand side string.
//   cfg        (const OmniPickerConfig&): [IN] The configuration values.
//   user_input (StringView)             : [IN] An optional string of keystrokes.
//
// [Returns]
//   (String): The selected history entries joined by spaces, or empty string if aborted.

String run_pid_picker(StringView lhs, const OmniPickerConfig& cfg, StringView user_input = "");
// Run the process ID picker TUI and return the selected entries joined by spaces.
//
// [Args]
//   lhs        (StringView)             : [IN] The left-hand side string.
//   cfg        (const OmniPickerConfig&): [IN] The configuration values.
//   user_input (StringView)             : [IN] An optional string of keystrokes.
//
// [Returns]
//   (String): The PIDs of the selected processes joined by spaces, or empty string if aborted.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
