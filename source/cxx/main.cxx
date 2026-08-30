////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: main.cxx                                                                    ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include STL headers.
#include <cstdint>
#include <iostream>
#include <stdexcept>

// Include the headers of custom modules.
#include "main_exbash.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main function
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{   // {{{

    // Make the STL I/O functions faster.
    std::cin.tie(nullptr);
    std::ios::sync_with_stdio(false);

    try
    {
        return main_exbash(argc, argv, "");
    }
    catch (const std::exception& e)
    {
        std::cerr << "ExBash: " << "\033[33mFatal\033[0m (main.cxx)" << std::endl;
        std::cerr << "-> Unhandled exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "ExBash: " << "\033[33mFatal\033[0m (main.cxx)" << std::endl;
        std::cerr << "-> Unknown non-std exception thrown" << std::endl;
        return EXIT_FAILURE;
    }

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
