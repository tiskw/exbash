////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: utils.cxx                                                                   ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the header.
#include "utils.hxx"

// Include the headers of STL.
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

String check_output(StringView command)
{   // {{{

    // Constants for timeout and output size limits.
    constexpr size_t MAX_OUTPUT_BYTES = 1024 * 1024;

    // Build the command string with a timeout of 1 second and redirect stderr to /dev/null.
    const String cmdstr = std::format("timeout -k 0.2s 1s sh -c {} 2>/dev/null", shlex_quote(command));

    // Use popen to execute the command and read its stdout.
    FILE* pipe = popen(cmdstr.c_str(), "r");
    if (not pipe) return "";

    // Initialize the output string and reserve space to avoid frequent reallocations.
    String result;
    result.reserve(4096);

    // Read from the pipe until EOF and append to the result string, ensuring we do not exceed MAX_OUTPUT_BYTES.
    char tmpbuf[4096];
    while (fgets(tmpbuf, sizeof(tmpbuf), pipe))
    {
        // Get the length of the read data.
        const size_t len = std::strlen(tmpbuf);

        // Calculate the remaining space in the result string before reaching the maximum output size.
        const size_t room = (result.size() < MAX_OUTPUT_BYTES) ? (MAX_OUTPUT_BYTES - result.size()) : 0;

        // If the length of the read data exceeds the remaining space, append only the allowed portion
        // and break the loop. The "break" statement jumps to the "pclose(pipe)" line and it may cause
        // a deadlock if the command is still writing to stdout. However, since the command is wrapped
        // with "timeout" command, the deadlock should not occur in practice.
        if (len > room) { result.append(tmpbuf, room); break; }

        // Append the read data to the result string.
        result.append(tmpbuf, len);
    }

    // Close the pipe and return the result string.
    pclose(pipe);
    return result;

}   // }}}

String expand_tilde(StringView path)
{   // {{{

    // Do nothing if the target path does not starts with "~".
    if (path.empty() or (path[0] != '~'))
        return String(path);

    // Get the value of the HOME environment variable and replace the leading "~" with it.
    const char* home = getenv("HOME");
    if (home)
        return String(home) + String(path.substr(1));

    return String(path);

}   // }}}

String get_perm_str(mode_t mode)
{   // {{{

    String s(10, '-');
    s[0] = S_ISDIR(mode)    ? 'd' : '-';
    s[1] = (mode & S_IRUSR) ? 'r' : '-';
    s[2] = (mode & S_IWUSR) ? 'w' : '-';
    s[3] = (mode & S_IXUSR) ? 'x' : '-';
    s[4] = (mode & S_IRGRP) ? 'r' : '-';
    s[5] = (mode & S_IWGRP) ? 'w' : '-';
    s[6] = (mode & S_IXGRP) ? 'x' : '-';
    s[7] = (mode & S_IROTH) ? 'r' : '-';
    s[8] = (mode & S_IWOTH) ? 'w' : '-';
    s[9] = (mode & S_IXOTH) ? 'x' : '-';
    return s;

}   // }}}

String get_size_str(uint64_t size)
{   // {{{

    constexpr auto fmt = [](double x, char unit) -> String
    // Format the size value with the appropriate unit and precision.
    //
    // [Args]
    //   x    (double): [IN] The size value to format.
    //   unit (char)  : [IN] The unit character ('K', 'M', 'G', 'T', 'P').
    //
    // [Returns]
    //   (String): The formatted size string with the specified unit.
    {
        if      (x <  10.0) { return std::format("{:.2f}{}", x, unit); }
        else if (x < 100.0) { return std::format("{:.1f}{}", x, unit); }
        else                { return std::format("{:.0f}{}", x, unit); }
    };

    // Convert the size to double for easier calculations.
    double x = static_cast<double>(size);

    for (int8_t k = 0; k < 5; ++k)
    {
        // If the size is greater than or equal to 1024, divide it by 1024 and go to the next unit.
        if (x >= 1024.0) { x /= 1024.0; continue; }

        switch (k)
        {
            case 0 : return pad_left(std::to_string(size) + "B", 5);
            case 1 : return pad_left(fmt(x, 'K'), 5);
            case 2 : return pad_left(fmt(x, 'M'), 5);
            case 3 : return pad_left(fmt(x, 'G'), 5);
            case 4 : return pad_left(fmt(x, 'T'), 5);
        }
    }

    // If the size is extremely large (greater than or equal to 1024^5), format it as petabytes.
    return pad_left(std::format("{:.0f}P", x), 5);

}   // }}}

Pair<String, Path> get_valid_path_of_last_token(StringView lhs)
{   // {{{

    // Do nothing if the input string is empty or ends with a space.
    if (lhs.empty() or (lhs.back() == ' '))
        return {String(lhs), Path(".")};

    // Split the input string into shell-like tokens.
    const Vector<StringView> tokens = shlex_split(lhs);

    // Do nothing if the input string is empty, ends with a space, or has no tokens.
    if (tokens.empty())
        return {String(lhs), Path(".")};

    const StringView last_token = tokens.back();
    const StringView others     = StringView(lhs.data(), last_token.data() - lhs.data());
    const Path       target     = Path(last_token);

    if (stdfs::exists(target)              ) { return {String(others), target              }; }
    if (stdfs::exists(target.parent_path())) { return {String(others), target.parent_path()}; }
    else                                     { return {String(others), Path(".")           }; }

}   // }}}

bool is_text_file(const Path& target)
{   // {{{

    // Return false (not a text file) if the target file does not exist.
    if (not stdfs::exists(target)) return false;

    // Open the file in binary mode.
    std::ifstream ifp(target, std::ios::binary);
    if (not ifp.is_open()) return false;

    // Read the first 1024 bytes of the file into a buffer.
    char buf[1024];
    ifp.read(buf, sizeof(buf));

    // Get the number of bytes actually read from the file.
    const std::streamsize n = ifp.gcount();

    // Check if any of the read bytes are null bytes ('\0').
    // If null byte is found, it is not a text file.
    for (std::streamsize i = 0; i < n; i++)
        if (buf[i] == '\0') return false;

    return true;

}   // }}}

String join(const Vector<String>& selected, const String& delim)
{   // {{{

    String result;
    for (const String& s : selected)
        result += result.empty() ? s : (delim + s);
    return result;

}   // }}}

String pad_left(String s, size_t width)
{   // {{{

    if (s.size() < width)
        s.insert(0, width - s.size(), ' ');
    return s;

}   // }}}

String pad_right(String s, size_t width)
{   // {{{

    if (s.size() < width)
        s.append(width - s.size(), ' ');
    return s;

}   // }}}

String read_file(const Path& path, int32_t max_bytes)
{   // {{{

    // Open the file in binary mode to read raw bytes without any encoding conversion.
    std::ifstream ifs(String(path), std::ios::binary);

    // Returns empty string if failed to open the file.
    if (not ifs) return "";

    if (max_bytes < 0)
        return String(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

    // Read the first `bytes_to_read` bytes from the file into a buffer.
    String buffer(max_bytes, '\0');
    ifs.read(buffer.data(), max_bytes);
    buffer.resize(static_cast<SizeType>(ifs.gcount()));

    return buffer;

}   // }}}

Generator<String> read_lines(const Path& path) noexcept
{   // {{{

    // Open the target file.
    std::ifstream ifp(path.c_str());

    // Do nothing if failed to open the target file.
    if (not ifp.is_open())
        co_return;

    // Reserve a temporary buffer.
    String line;

    while (std::getline(ifp, line))
        co_yield line;

    ifp.close();

}   // }}}

String shlex_quote(StringView s)
{   // {{{

    constexpr auto is_safer_char = [](unsigned char c) -> bool
    // Check if the character is safe to use in a shell command without quoting.
    //
    // [Args]
    //   c (unsigned char): [IN] The character to check.
    //
    // [Returns]
    //   (bool): True if the character is alphanumeric or one of the safe characters, false otherwise.
    {
        constexpr StringView safe_chars = "_-./:@+=";
        return std::isalnum(c) or (safe_chars.find(static_cast<char>(c)) != StringView::npos);
    };

    // If the string is empty, return ''.
    if (s.empty()) return "''";

    // Check each character in the string to see if it is safe.
    // If any character is not safe, we need to quote the string.
    for (const unsigned char c : s)
    {
        // If any character is not safe, we need to quote the string.
        if (is_safer_char(c)) continue;

        // Initialize the result string.
        String r = "'";

        for (const char ch : s)
        {
            if (ch == '\'') { r += "'\\''"; }
            else            { r += ch;      }
        }

        // Close the single quote and return the quoted string.
        return r + "'";
    }

    return String(s);

}   // }}}

String shlex_join(const Vector<String>& args)
{   // {{{

    String result;
    for (const String& a : args)
        result += String(result.empty() ? "" : " ") + shlex_quote(a);
    return result;

}   // }}}

Vector<StringView> shlex_split(StringView sv)
{   // {{{

    // Initialize the result vector to hold the split tokens.
    Vector<StringView> result;

    SizeType idx_last = 0; //
    SizeType idx_curr = 0; //

    bool in_single = false; // True if we are inside single quotes.
    bool in_double = false; // True if we are inside double quotes.
    bool backslash = false; // True if the previous character was a backslash.

    for (const char c : sv)
    {
        // Case 1: If the previous character was a backslash, add the current
        //         character to the current token and reset the backslash flag.
        if (backslash) { backslash = false; }

        // Case 2: Update the state flags.
        else if ((c == '\\') and not in_single) { backslash = true;       }
        else if ((c == '\'') and not in_double) { in_single = !in_single; }
        else if ((c == '"' ) and not in_single) { in_double = !in_double; }

        // Case 3: If we encounter whitespace (space or tab) and we are
        //         not inside quotes, then we treat it as a token separator.
        else if ((c == ' ' || c == '\t') and (not in_single) and (not in_double))
        {
            // If the target token is empty (i.e. idx_last and idx_curr is the same), skip adding to the result.
            if (idx_last != idx_curr)
                result.push_back(StringView(sv.data() + idx_last, idx_curr - idx_last));

            // Update the index of the beginning of the next target token.
            idx_last = idx_curr + 1;
        }

        // Update the index of the current character.
        ++idx_curr;
    }

    // After processing all characters, if there is a non-empty current token, add it to the result.
    if (idx_last != idx_curr)
        result.push_back(StringView(sv.data() + idx_last, idx_curr - idx_last));

    return result;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
