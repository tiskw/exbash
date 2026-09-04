////////////////////////////////////////////////////////////////////////////////////////////////////
// C++ source file: utils.cxx                                                                    ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "utils.hxx"

// Include STL headers.
#include <fstream>

// Include POSIX headers.
#include <sys/ioctl.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////

String expand_tilde(StringView path)
{   // {{{

    // Get home directory from the environment variable "HOME".
    const char* home = std::getenv("HOME");

    // Replace "~" with the home directory if the path starts with "~" and the "home" is not null.
    if (path.starts_with("~") and (home != nullptr))
        return String(home) + String(path.substr(1));

    // Otherwise, do nothing.
    return String(path);

}   // }}}

Size get_terminal_size(void) noexcept
{   // {{{

    struct winsize ws;

    // Get the terminal size.
    // If failed to get the size, return the default size (24 rows and 80 columns).
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0)
        return {80, 24};

    return {ws.ws_col, ws.ws_row};

}   // }}}

String get_time(time_t raw_time, const char* format) noexcept
{   // {{{

    // Convert "time_t" to "struct tm".
    struct tm time_info;
    localtime_r(&raw_time, &time_info);

    // Convert to string.
    std::stringstream ss;
    ss << std::put_time(&time_info, format);

    return ss.str();

}   // }}}

Generator<String> readline(const char* path) noexcept
{   // {{{

    // Open the target file.
    std::ifstream ifp(path);

    // Do nothing if failed to open the target file.
    if (not ifp.is_open())
        co_return;

    // Reserve a temporary buffer.
    String line;

    while (std::getline(ifp, line))
        co_yield line;

    ifp.close();

}   // }}}

String replace(StringView target, StringView oldstr, StringView newstr) noexcept
{   // {{{

    // Create copy of the input string.
    String replaced(target);

    // Reserve the size of the replaced string to avoid multiple memory allocations.
    if (newstr.size() > oldstr.size())
        replaced.reserve(target.size() + (newstr.size() - oldstr.size()) * 8);

    // Find the old string in the target string.
    SizeType pos = replaced.find(oldstr);
 
    while (pos != String::npos)
    {
        // Replace the old string to the new string.
        replaced.replace(pos, oldstr.length(), newstr);

        // Find the old string again.
        pos = replaced.find(oldstr, pos + newstr.length());
    }

    // Returns the replaced string.
    return replaced;

}   // }}}

Generator<StringView> split(StringView sv, StringView delim) noexcept
{   // {{{

    // Do nothing if the delimiter is an empty string.
    if (delim.size() == 0) { co_yield sv; co_return; }

    // Initialize offset which indicates current position.
    SizeType offset = 0;

    while (true)
    {
        // Find next target.
        const SizeType pos = sv.find(delim, offset);

        // Returns if no next target found.
        if (pos == String::npos)
        {
            co_yield StringView(sv.begin() + offset, sv.end());
            co_return;
        }

        // Otherwise, memorize the found target and update the offset value.
        co_yield StringView(sv.data() + offset, pos - offset);
        offset = pos + delim.size();
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

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
