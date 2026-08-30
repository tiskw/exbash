////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: test_utils.cxx                                                           ///
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
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test function
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void)
{   // {{{

    std::setlocale(LC_ALL, "");

    // utils: inline helpers
    assert(clip(5, 0, 10) == 5);
    assert(clip(-1, 0, 10) == 0);
    assert(clip(99, 0, 10) == 10);
    assert(hash("abc") == hash(StringView("abc")));
    assert(hash(static_cast<const char*>(nullptr)) == 0xcbf29ce484222325ULL);

    // utils: shell quoting and splitting
    assert(shlex_quote("") == "''");
    assert(shlex_quote("abc-./:@+_=AZ09") == "abc-./:@+_=AZ09");
    assert(shlex_quote("a b") == "'a b'");
    assert(shlex_quote("a'b") == "'a'\\''b'");
    assert(shlex_join(Vector<String>{"abc", "a b", "a'b"}) == "abc 'a b' 'a'\\''b'");
    const Vector<StringView> tokens = shlex_split("cmd 'two words' \"dq words\" a\\ b plain");
    assert(tokens.size() == static_cast<size_t>(5));
    assert(tokens[0] == "cmd");
    assert(tokens[1] == "two words");
    assert(tokens[2] == "dq words");
    assert(tokens[3] == "a b");
    assert(tokens[4] == "plain");

    // utils: size strings
    assert(get_size_str(0) == "0    ");
    assert(get_size_str(1) == "1    ");
    assert(get_size_str(1023) == "1023 ");
    assert(get_size_str(1024) == "1.00K");
    assert(get_size_str(10 * 1024) == "10.0K");
    assert(get_size_str(100 * 1024) == " 100K");
    assert(get_size_str(1024ULL * 1024ULL) == "1.00M");

    // utils: permission strings
    assert(get_perm_str(0000) == "----------");
    assert(get_perm_str(0400) == "-r--------");
    assert(get_perm_str(0200) == "--w-------");
    assert(get_perm_str(0100) == "---x------");
    assert(get_perm_str(0070) == "----rwx---");
    assert(get_perm_str(0007) == "-------rwx");

    // expand_tilde
    const String home = std::getenv("HOME");
    assert(expand_tilde("~") == home);
    assert(expand_tilde("~/abc") == home + "/abc");
    assert(expand_tilde("/tmp/abc") == "/tmp/abc");

    // utils: file I/O and text/binary detection
    const Path text_file = "./test_utils.cxx";
    const Path bin_file  = "./test_utils";
    assert(read_file(text_file, 5) == "/////");
    assert(read_file(text_file).starts_with("//////////"));
    assert(read_file("missing.txt") == "");
    assert(is_text_file(text_file));
    assert(not is_text_file(bin_file));
    assert(not is_text_file("missing.txt"));

    // utils: read_lines
    int32_t line_count = 0;
    for (const String& line : read_lines(text_file))
        if (line_count++ == 0)
            assert(line.starts_with("//////////"));

    // utils: last-token path extraction
    auto r1 = get_valid_path_of_last_token("cat a.txt");
    assert(r1.first == "cat ");
    assert(r1.second.filename().string() == "a.txt");
    auto r2 = get_valid_path_of_last_token("cat missing/name.txt");
    assert(r2.first == "cat ");
    assert(r2.second.string() == ".");
    auto r3 = get_valid_path_of_last_token("cat dirA/new.txt");
    assert(r3.first == "cat ");
    assert(r3.second.filename().string() == "dirA");
    auto r4 = get_valid_path_of_last_token("cat ");
    assert(r4.first == "cat ");
    assert(r4.second.string() == ".");

    // utils: command output wrapper
    assert(check_output("printf hello") == "hello");
    assert(check_output("printf out; printf err >&2") == "out");

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
