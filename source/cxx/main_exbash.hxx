////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: main_exbash.hxx                                                             ///
///                                                                                              ///
/// Main function of ExBash.                                                                     ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MAIN_EXBASH_HXX
#define MAIN_EXBASH_HXX

// Include STL headers.
#include <cstdint>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

inline constexpr const char* SOFTWARE_NAME = "exbash";
inline constexpr const char* SOFTWARE_DESC = "Lightweight alternative to GNU Readline for ExBash.";

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function definitioins
////////////////////////////////////////////////////////////////////////////////////////////////////

int32_t main_exbash(int32_t argc, char* argv[], const char* input_str);
// Actual main function of ExBash.
//
// [Args]
//   argc      (int)         : [IN] The number of command line arguments.
//   argv      (char*[])     : [IN] The array of command line arguments.
//   input_str (const char*) : [IN] User input (for debugging).
//
// [Returns]
//   (int32_t): Return code.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
