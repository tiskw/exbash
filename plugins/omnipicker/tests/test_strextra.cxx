////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: test_strextra.cxx                                                           ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the headers of STL.
#include <algorithm>
#include <clocale>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Include POSIX headers.
#include <unistd.h>

// Include the test header.
#include "test_common.hxx"

// Include the headers of custom modules.
#include "dtypes.hxx"
#include "strextra.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions used for the test
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
    std::vector<String> collect_split_ansi(const String& s)
    {   // {{{

        std::vector<String> v;
        for (const auto& token : split_ansi(s))
            v.push_back(token);
        return v;

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test function
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void)
{   // {{{

    std::setlocale(LC_ALL, "");

    //
    assert(clipstr("abcdef", 0) == "");
    assert(clipstr("abcdef", -4) == "");
    assert(clipstr("abcdef", 3) == "abc");
    assert(clipstr("aあb", 1) == "a");
    assert(clipstr("aあb", 2) == "a");
    assert(clipstr("aあb", 3) == "aあ");
    assert(clipstr("あいう", 4) == "あい");

    //
    assert(is_ansi("\x1b[0m"));
    assert(is_ansi("\x1b[31m"));
    assert(not is_ansi(""));
    assert(not is_ansi("[31m"));
    assert(not is_ansi("\x1b[31K"));

    //
    assert(String(remove_last_utf8_char("abc")) == "ab");
    assert(String(remove_last_utf8_char("aあ")) == "a");
    assert(String(remove_last_utf8_char("")) == "");

    // assert(parse_ansi("\x1b[0m") == 0);
    // assert(parse_ansi("\x1b[m") == 0);
    // assert(parse_ansi("\x1b[31m") == 31);
    // assert(parse_ansi("\x1b[1;32m") == 32);
    // assert(parse_ansi("\x1b[38;5;1m") == 0);
    // assert(parse_ansi("not ansi") == 0);

    //
    const auto parts = collect_split_ansi(String("A") + "\x1b[31m" + "B" + "\x1b[0m" + "C");
    assert(parts.size() == static_cast<size_t>(5));
    assert(parts[0] == "A");
    assert(parts[1] == "\x1b[31m");
    assert(parts[2] == "B");
    assert(parts[3] == "\x1b[0m");
    assert(parts[4] == "C");

    //
    assert(strwidth("abc") == 3);
    assert(strwidth("aあb") == 4);
    assert(strwidth(String("\x1b[31m") + "赤" + "\x1b[0m") == 2);

    // Print header of overall result.
    std::cout                                         << std::endl;
    std::cout << "=============================="     << std::endl;
    std::cout << "\033[33mOVERALL TEST RESULT\033[0m" << std::endl;

    // Print test result.
    if (passed) { std::cout << "\033[32mPASSED\033[0m" << std::endl; }
    else        { std::cout << "\033[31mFAILED\033[0m" << std::endl; }

    return EXIT_SUCCESS;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
