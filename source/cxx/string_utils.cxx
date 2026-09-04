////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: string_utils.cxx                                                            ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "string_utils.hxx"

// Include the headers of custom modules.
#include "utf8.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local static variables and functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    // ANSI escape codes for colors.
    constexpr const char* COLOR_COMMAND = "\x1B[32m";
    constexpr const char* COLOR_KEYWORD = "\x1B[33m";
    constexpr const char* COLOR_SYMBOL  = "\x1B[34m";
    constexpr const char* COLOR_STRING  = "\x1B[31m";
    constexpr const char* COLOR_RESET   = "\x1B[0m";

    // Set of commands, keywords, and symbols to be colorized.
    const Set<StringView> set_commands = {
        "cat", "cd", "chmod", "chown", "cp", "echo", "env", "export", "grep", "let", "ln", "ls",
        "make", "mkdir", "mv", "rm", "sed", "set", "tar", "touch", "umask", "unset",
    };
    const Set<StringView> set_keywords = {
        "case", "do", "done", "elif", "else", "esac", "exit", "fi", "for", "function", "if", "in",
        "local", "read", "return", "select", "shift", "then", "time", "until", "while",
    };
    const Set<StringView> set_symbols = {
        "&", "|", ">", "<", "&&", "||", ">>", "<<",
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Generator<StringView> chunk(StringView sv, uint16_t width)
{   // {{{

    // Edge cases: the given width is zero, or the colorized string has zero width.
    if (width == 0) { co_yield sv; co_return; }

    // A variable to store the starting index of the current chunk.
    SizeType chunk_start = 0;

    // A variable to store the current width of the chunk.
    uint16_t chunk_width = 0;

    for (const StringView ch : utf8_iter(sv.data(), sv.size()))
    {
        // Skip the ANSI escape sequence.
        if ((ch.size() > 1) and (*ch.data() == '\x1B') and (*(ch.data() + 1) == '['))
            continue;

        // Decode the current UTF-8 character to get its codepoint.
        int32_t codepoint = -1;
        utf8_decode(reinterpret_cast<const uint8_t*>(ch.data()), ch.size(), &codepoint);

        // Get the width of the current UTF-8 character.
        uint8_t char_width = (codepoint >= 0) ? static_cast<uint8_t>(utf8_width(codepoint)) : 0;

        // If adding the current character exceeds the given width, yield the current chunk and start a new chunk.
        if (chunk_width + char_width > width)
        {
            // Yield the current chunk as a string view.
            co_yield StringView(sv.data() + chunk_start, ch.data() - (sv.data() + chunk_start));

            // Start a new chunk from the current character.
            chunk_start = ch.data() - sv.data();
            chunk_width = 0;
        }

        // Add the width of the current character to the current chunk width.
        chunk_width += char_width;
    }

    // Yield the last chunk if it exists.
    if (chunk_start < sv.size())
        co_yield StringView(sv.data() + chunk_start, sv.size() - chunk_start);

    co_return;

}   // }}}

StringView textclip(StringView sv, uint16_t width)
{   // {{{

    // Edge cases: the given width is zero, or the colorized string has zero width.
    if (width == 0) return sv;

    // Initialize the total width of the clipped string.
    uint16_t clip_width = 0;

    for (const auto& [codepoint, ptr] : utf8_decode_iter(sv.data(), sv.size()))
    {
        // Get the width of the current UTF-8 character.
        uint8_t char_width = (codepoint >= 0) ? static_cast<uint8_t>(utf8_width(codepoint)) : 0;

        // If adding the current character exceeds the given width, return the clipped string up to the previous character.
        if (clip_width + char_width > width)
            return StringView(sv.data(), reinterpret_cast<const char*>(ptr) - sv.data());

        // Add the width of the current character to the total clipped width.
        clip_width += char_width;
    }

    // If the loop completes without exceeding the width, return the original string.
    return sv;

}   // }}}

String colorize(StringView sv)
{   // {{{

    // Initialize the colorized string and reserve enough size to avoid multiple reallocations.
    String c_str;
    c_str.reserve(2 * sv.size());

    // A flag to indicate whether the current position is in a string literal,
    // and a variable to store the starting character of the string literal.
    bool in_string_literal  = false;
    char string_quote_start = '\0';

    // A variable to store the current word being processed.
    String word;
    word.reserve(32);

    for (const char c : sv)
    {
        // Convert the character to unsigned char (for the std::isspace function below).
        const unsigned char uc = static_cast<unsigned char>(c);

        // Break the loop if the null character is encountered.
        if (c == '\0')
            break;

        // Case 1: End of a string literal.
        else if (in_string_literal and (c == string_quote_start))
        {
            // Append the closing quote and reset the color.
            c_str += c;
            c_str += COLOR_RESET;

            // Reset the string literal flag and the starting character.
            in_string_literal  = false;
            string_quote_start = '\0';
        }

        // Case 2: Inside a string literal.
        else if (in_string_literal)
            c_str += c;

        // Case 3: Start of a string literal.
        else if ((c == '\'') or (c == '"'))
        {
            // Flush any in-progress word before starting the string literal,
            // so that the word appears before the opening quote in the output.
            if (not word.empty())
            {
                if      (set_commands.contains(word)) { c_str += COLOR_COMMAND; }
                else if (set_keywords.contains(word)) { c_str += COLOR_KEYWORD; }
                else if (set_symbols .contains(word)) { c_str += COLOR_SYMBOL;  }
                c_str += word;
                c_str += COLOR_RESET;
                word.clear();
            }

            // Append the color code and the opening quote.
            c_str += COLOR_STRING;
            c_str += c;

            // Set the string literal flag and the starting character.
            in_string_literal  = true;
            string_quote_start = c;
        }

        // Case 4: A whitespace character before any word.
        else if (word.empty() and std::isspace(uc))
            c_str += c;

        // Case 5: A non-whitespace character (part of a word).
        else if (not std::isspace(uc))
            word += c;

        // Case 6: A whitespace character after a word (end of a word).
        else
        {
            // Append the color code for the current word if it is a reserved keyword.
            if      (set_commands.contains(word)) { c_str += COLOR_COMMAND; }
            else if (set_keywords.contains(word)) { c_str += COLOR_KEYWORD; }
            else if (set_symbols .contains(word)) { c_str += COLOR_SYMBOL;  }

            // Append the current word and reset the color, then append the current character (= whitespace).
            c_str += word;
            c_str += COLOR_RESET;
            c_str += c;

            // Clear the appended word for the next word.
            word.clear();
        }
    }

    // Handle the last word if the input string does not end with a whitespace character.
    if (not word.empty())
    {
        if      (set_commands.contains(word)) { c_str += COLOR_COMMAND; }
        else if (set_keywords.contains(word)) { c_str += COLOR_KEYWORD; }
        else if (set_symbols .contains(word)) { c_str += COLOR_SYMBOL;  }

        c_str += word;
        c_str += COLOR_RESET;
    }

    // If the input string ends while still in a string literal, reset the color
    // to avoid polluting the other areas of the terminal.
    if (in_string_literal)
        c_str += COLOR_RESET;

    return c_str;

}   // }}}

String insert_cursor(StringView sv, uint16_t cursor_pos)
{   // {{{

    // Edge case: the input string is empty.
    if (sv.empty()) return "\x1B[7m \x1B[27m";

    // Initialize the output string and reserve enough size to avoid multiple reallocations.
    String result;
    result.reserve(sv.size() + 32);

    // A variable to store the current width of the string being processed.
    uint16_t current_width = 0;

    // A flag to indicate whether the ANSI escape sequence for reverse is currently open.
    bool is_reversed = false;

    for (const StringView ch : utf8_iter(sv.data(), sv.size()))
    {
        if ((ch.size() >= 2) and (ch[0] == '\x1B') and (ch[1] == '['))
        {
            // Append the ANSI escape sequence to the result string.
            result += ch;
        }
        else
        {
            // If the current width matches the cursor position, append the ANSI escape code for reverse.
            if (current_width == cursor_pos)
            {
                result += "\x1B[7m";
                is_reversed = true;
            }

            // Append the ANSI escape code to reset the reverse effect if it is currently open and the current width exceeds the cursor position.
            if (is_reversed and (current_width > cursor_pos))
            {
                result += "\x1B[27m";
                is_reversed = false;
            }

            // Append the current character to the result string.
            result += ch;

            // Decode the current UTF-8 character to get its codepoint.
            int32_t codepoint;
            utf8_decode(reinterpret_cast<const uint8_t*>(ch.data()), ch.size(), &codepoint);

            // Add the width of the current character to the current width.
            current_width += (codepoint >= 0) ? static_cast<uint16_t>(utf8_width(codepoint)) : 0;
        }
    }

    // If the ANSI escape sequence for reverse is still open after processing the entire string, append the ANSI escape code to reset it.
    if (is_reversed)
        result += "\x1B[27m";

    // If the cursor position is at the end of the string, append the cursor symbol at the end.
    if (current_width == cursor_pos)
        result += "\x1B[7m \x1B[27m";

    return result;

}   // }}}

uint16_t width(StringView sv)
{   // {{{

    // Safeguard: If the size of the input string is zero, then the width of the string should be zero.
    if (sv.size() == 0) return 0;

    // Initialize the output variable.
    uint16_t total_width = 0;

    // Sum up the widths of all UTF-8 characters in the colorized string.
    for (const auto& [codepoint, ptr] : utf8_decode_iter(sv.data(), sv.size()))
        total_width += (codepoint > 0) ? utf8_width(codepoint) : 0;

    return total_width;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
