////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: error.cxx                                                                   ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "error.hxx"

// Include STL headers.
#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local variables
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    // A flag indicating whether the TUI is active or not.
    bool is_tui_active = false;

    // A vector to store pending error messages.
    Vector<String> pending_errors;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

int32_t print_errmsg(const char (&etype)[], const char (&filename)[], int32_t line_no, const char (&funcname)[], StringView msg, bool terminate)
{   // {{{

    std::stringstream ss;
    ss << "ExBash: " << "\033[33m" << etype << "\033[0m (" << filename << ", L." << line_no << ", in " << funcname << ")" << std::endl;
    ss << "-> " << msg << std::endl;

    // If the TUI is active, store the error message in the pending_errors vector.
    // Otherwise, print the error message to the standard error stream.
    if (is_tui_active) { pending_errors.push_back(ss.str()); }
    else               { std::cerr << ss.str();              }

    // Terminate the software if "terminate" is true.
    if (terminate)
        throw std::runtime_error("ExBash: error");

    // Returns failure code.
    return EXIT_FAILURE;

}   // }}}

void set_tui_active(bool active)
{   // {{{

    // Update the TUI active flag.
    is_tui_active = active;

    // If the TUI is not active and there are pending error messages, print them.
    if ((not is_tui_active) and (not pending_errors.empty()))
    {
        std::cerr << "ExBash: " << pending_errors.size() << " pending error messages:" << std::endl;
        for (const String& err : pending_errors)
            std::cerr << err;
        pending_errors.clear();
    }

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
