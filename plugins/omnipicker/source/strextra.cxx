////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: strextra.cxx                                                                ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the header.
#include "strextra.hxx"

// Include the headers of STL.
#include <algorithm>
#include <cwchar>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// File-local helper types and functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    struct CharInfo
    // Holds information about one Unicode codepoint within a UTF-8 string.
    {
        SizeType byte_start;    // Byte offset of this character in the original string.
        SizeType byte_len;      // Number of bytes this character occupies.
        uint32_t codepoint;     // Unicode codepoint value.
        int      display_width; // Display width: 1 for narrow, 2 for wide (CJK etc.).
    };

    Generator<CharInfo> get_char_info(StringView text)
    // Parse a UTF-8 string and return per-character information.
    //
    // [Args]
    //   text (StringView) [IN] The input UTF-8 string.
    //
    // [Returns]
    //   (Vector<CharInfo>): A vector of CharInfo, one for each Unicode
    //
    {   // {{{

        SizeType i = 0;

        while (i < text.size())
        {
            CharInfo ci;

            // Determine the byte offset of this character in the original string.
            ci.byte_start = i;

            // Determine the number of bytes and the Unicode codepoint value for this character.
            const unsigned char c = static_cast<unsigned char>(text[i]);
            if      (c < 0x80) { ci.codepoint = c & 0x7F; ci.byte_len = 1; }
            else if (c < 0xE0) { ci.codepoint = c & 0x1F; ci.byte_len = 2; }
            else if (c < 0xF0) { ci.codepoint = c & 0x0F; ci.byte_len = 3; }
            else               { ci.codepoint = c & 0x07; ci.byte_len = 4; }

            // Combine the continuation bytes to form the full Unicode codepoint.
            for (SizeType j = 1; j < ci.byte_len && (i + j) < text.size(); j++)
                ci.codepoint = (ci.codepoint << 6) | (static_cast<unsigned char>(text[i + j]) & 0x3F);

            // Determine the display width of this character using wcwidth.
            ci.display_width = std::max(0, wcwidth(static_cast<wchar_t>(ci.codepoint)));

            co_yield ci;

            // Update the byte offset for the next character.
            i += ci.byte_len;
        }

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

String clipstr(StringView text, int32_t width)
{   // {{{

    // If the width is zero or negative, return an empty string.
    if (width <= 0) return "";

    // Iterate through the characters and accumulate display width until we reach the limit.
    int32_t  accum    = 0;
    SizeType cut_byte = 0;

    for (const CharInfo& ci : get_char_info(text))
    {
        // If adding this character would exceed the width limit, stop.
        if (accum + ci.display_width > width)
            break;

        // Otherwise, include this character and update the accum and cut_byte.
        accum   += ci.display_width;
        cut_byte = ci.byte_start + ci.byte_len;
    }

    // Return the substring of the original text up to the cut_byte offset.
    return String(text.substr(0, cut_byte));

}   // }}}

String fitstr(StringView text, int32_t width)
{   // {{{

    // If the width is zero or negative, return an empty string.
    if (width <= 0) return "";

    // Clip the string to fit within the specified width.
    String clipped = clipstr(text, width);

    // Calculate the display width of the clipped string.
    const int32_t clipped_width = strwidth(clipped);

    // If the clipped string is shorter than the desired width, pad it with spaces on the right.
    if (clipped_width < width)
        clipped.append(width - clipped_width, ' ');

    return clipped;

}   // }}}

bool is_ansi(StringView sv)
{   // {{{

    // Check if the string is at least 3 characters long and starts with "\x1B[" and ends with "m".
    return (sv.size() >= 3) and (sv[0] == '\x1B') and (sv[1] == '[') and (sv.back() == 'm');

}   // }}}

StringView remove_last_utf8_char(StringView sv)
{   // {{{

    // If the string is empty, do nothing.
    if (sv.empty()) return sv;

    // Walk backwards past any UTF-8 continuation bytes (0x80–0xBF).
    SizeType idx = sv.size() - 1;
    while ((idx > 0) and ((static_cast<unsigned char>(sv[idx]) & 0xC0) == 0x80))
        --idx;

    return sv.substr(0, idx);

}   // }}}

Generator<int32_t> parse_ansi(const String& code)
{   // {{{

    // If the code is too short to be a valid ANSI escape sequence, return 0.
    if (code.size() < 3) co_return;

    // Check if the code starts with the ANSI escape sequence prefix "\x1B[" and ends with "m".
    if ((code[0] != '\x1B') or (code[1] != '[') or (code.back() != 'm')) co_return;

    // Extract the inner part of the ANSI code (between "\x1B[" and "m").
    const String inner = code.substr(2, code.size() - 3);

    // Empty code means reset.
    if (inner.empty())
    {
        co_yield 0;
        co_return;
    }

    // Split the inner part by semicolons to get individual numeric codes.
    std::istringstream ss(inner);
    String num;
    while (std::getline(ss, num, ';'))
    {
        try
        {
            const int32_t n = std::stoi(num);
            if ( n == 0                ) co_yield 0;
            if ((30 <= n) and (n <= 37)) co_yield n;
        }
        catch (...)
        { /* Just ignore any parsing errors and continue to the next code. */ }
    }

}   // }}}

Generator<String> split_ansi(const String& text)
{   // {{{

    static const std::regex ansi_re("(\x1B\\[[0-9;]*m|[^\x1B]+)");
    for (std::sregex_token_iterator it(text.begin(), text.end(), ansi_re, {0}), iter_end; it != iter_end; ++it)
        co_yield it->str();

}   // }}}

Generator<Tuple<int32_t, String>> split_lines(const String& str)
{   // {{{

    // Convert the input string into a string stream.
    std::istringstream ss(str);

    // Initialize the line index and a temporary string.
    int32_t idx = 0;
    String  line;

    while (std::getline(ss, line))
    {
        // Strip extra whitespace from the line.
        line = strip(line);

        // Yield the line along with its index if it is not empty.
        if (not line.empty())
            co_yield {idx++, line};
    }

}   // }}}

StringView strip(StringView sv, bool left, bool right) noexcept
{   // {{{

    constexpr auto is_not_space = [](const char ch) noexcept -> bool
    // Returns true if the given character is not whitespace.
    //
    // [Args]
    //   ch (unsigned char): [IN] Target character.
    //
    // [Returns]
    //   (bool): True if not a space.
    {
        return !std::isspace(static_cast<unsigned char>(ch));
    };

    PtrDiff idx_bgn = left  ? std::find_if(sv.begin(),  sv.end(),  is_not_space)        - sv.begin() : 0;
    PtrDiff idx_end = right ? std::find_if(sv.rbegin(), sv.rend(), is_not_space).base() - sv.begin() : sv.size();

    // Return the stripped string view if there is any non-space character, otherwise return an empty string view.
    return (idx_end > idx_bgn) ? StringView(sv.data() + idx_bgn, idx_end - idx_bgn) : StringView(sv.data(), 0);

}   // }}}

int32_t strwidth(const String& text)
{   // {{{

    // Define a regular expression to match ANSI SGR escape sequences.
    static const RegEx ansi_re("\x1B\\[[0-9;]*m");

    // Remove ANSI escape sequences from the text to get the clean string.
    const String cleaned = std::regex_replace(text, ansi_re, "");

    // Calculate the total display width of the clean string.
    int32_t total = 0;
    for (const CharInfo& ci : get_char_info(cleaned))
        total += ci.display_width;

    return total;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
