////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: error.cxx                                                                   ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "error.hxx"

// Include STL headers.
#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

int32_t print_errmsg(const char (&etype)[], const char (&filename)[], int32_t line_no, const char (&funcname)[], StringView msg, bool terminate)
{   // {{{

    // Print error info.
    std::cerr << "ExBash: " << "\033[33m" << etype << "\033[0m (" << filename << ", L." << line_no << ", in " << funcname << ")" << std::endl;
    std::cerr << "-> " << msg << std::endl;

    // Terminate the software if "terminate" is true.
    if (terminate)
        throw std::runtime_error("ExBash: error");

    // Returns failure code.
    return EXIT_FAILURE;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
