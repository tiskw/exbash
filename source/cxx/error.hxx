////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: error.hxx                                                                   ///
///                                                                                              ///
/// Print error messages.                                                                        ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ERROR_HXX
#define ERROR_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////////////////////////

#define print_error(etype, msg) print_errmsg(etype, __FILE__, __LINE__, __func__, msg, false)
// Wrapper of print_errmsg macro for printing error message without terminating the software.
//
// [Args]
//   etype (const char*)  : [IN] Error type.
//   msg   (const String&): [IN] Error message to show.
//
// [Returns]
//   (int32_t): EXIT_FAILURE.

#define print_error_and_exit(etype, msg)  print_errmsg(etype, __FILE__, __LINE__, __func__, msg, true)
// Wrapper of print_errmsg macro for printing error message and terminating the software.
//
// [Args]
//   etype (const char*)  : [IN] Error type.
//   msg   (const String&): [IN] Error message to show.
//
// [Returns]
//   (int32_t): EXIT_FAILURE.

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

int32_t print_errmsg(const char (&etype)[], const char (&filename)[], int32_t line_no, const char (&func_name)[], StringView msg, bool terminate);
// Print error message.
//
// [Args]
//   etype     (const char&[]): [IN] Error type.
//   filename  (const char&[]): [IN] File name where the error occurred.
//   line_no   (int32_t)      : [IN] Line number where the error occurred.
//   funcname  (const char&[]): [IN] Function name number where the error occurred.
//   msg       (StringView)   : [IN] Error message to show.
//   terminate (bool)         : [IN] Terminate the software if true.
//
// [Notes]
//   The argument "etype", "filename", and "funcname" accept only string literals.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
