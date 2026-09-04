////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: test_common.cxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the headers of STL.
#include <algorithm>
#include <clocale>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Include POSIX headers.
#include <unistd.h>

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////////////////////////////////

static bool passed = true;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility macros and functions for testing
////////////////////////////////////////////////////////////////////////////////////////////////////

#define assert(expr) (assert_body((expr), #expr, __LINE__))

static inline void assert_body(bool expr_bool, const char* expr_str, int line_no)
{   // {{{

    // Print test result.
    if (expr_bool) std::cout << "\033[32mPASSED\033[0m";
    else           std::cout << "\033[31mFAILED\033[0m";

    // Print detailed information.
    std::cout << ": L." << line_no << ": " << expr_str << std::endl;

    // Update the passed/failed flag.
    passed &= expr_bool;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
