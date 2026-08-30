////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: utils.hxx                                                                   ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef UTILS_HXX
#define UTILS_HXX

// Include the headers of STL.
#include <algorithm>

// Include POSIX headers.
#include <sys/stat.h>

// Include custom headers.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Inline template or non-template functions
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
T clip(T x, T x_min, T x_max)
// Clamp x to the range [x_min, x_max].
//
// [Args]
//   x     (T): [IN] The value to be clamped.
//   x_min (T): [IN] The minimum allowed value.
//   x_max (T): [IN] The maximum allowed value.
//
// [Returns]
//   (T): The clamped value.
//
{   // {{{

    return std::max(x_min, std::min(x, x_max));

}   // }}}

inline constexpr uint64_t hash(StringView str, uint64_t hash_init = 0xcbf29ce484222325)
// Compute hash value of the given string.
//
// [Args]
//   str (StringView): [IN] The target string.
//
// [Returns]
//   (uint64_t): Hash value of the given string.
//
// [Notes]
//   This is FNV-1a algorithm, a simple non-cryptographic hash function.
//   FNV-1a is fast and simple to implement, but has a higher collision rate than sha1/md5.
//
{   // {{{

    // Define the prime value.
    constexpr uint64_t prime = 0x100000001b3;

    // Initialize the hash value.
    uint64_t hash = hash_init;

    // Update the hash value.
    for (const char c : str)
        hash = prime * (hash ^ c);

    return hash;

};  // }}}

inline constexpr uint64_t hash(const char* str, uint64_t hash_init = 0xcbf29ce484222325)
// Compute hash value of the given string.
//
// [Args]
//   str (const char*): [IN] The target string.
//
// [Returns]
//   (uint64_t): Hash value of the given string.
//
// [Notes]
//   This is FNV-1a algorithm, a simple non-cryptographic hash function.
//   FNV-1a is fast and simple to implement, but has a higher collision rate than sha1/md5.
//
{   // {{{

    // Returns the initial hash value if the input string is nullptr.
    if (str == nullptr)
        return hash_init;

    return hash(StringView(str), hash_init);

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

String check_output(StringView command);
// Run a shell command and return its stdout as a string (stderr suppressed).
//
// [Args]
//   command (const String&): [IN] The shell command to execute.
//
// [Returns]
//   (String): The stdout output of the command, or an empty string if execution fails.

String expand_tilde(StringView path);
// Expand leading "~" to the home directory.
//
// [Args]
//   path (StringView): [IN] The input path string, which may start with "~".
//
// [Returns]
//   (String): The expanded path string.

String get_perm_str(mode_t mode);
// Build a Unix permission string like "drwxr-xr-x".
//
// [Args]
//   mode (mode_t): [IN] The st_mode field from struct stat.
//
// [Returns]
//   (String): The permission string representing the file type and permissions.

String get_size_str(uint64_t size);
// Return a human-readable file size string, right-justified in 5 characters.
//
// [Args]
//   size (uint64_t): [IN] The file size in bytes.
//
// [Returns]
//   (String): The formatted size string.

Pair<String, Path> get_valid_path_of_last_token(StringView lhs);
// Extract the last shell token from lhs and resolve it as a filesystem path.
//
// [Args]
//   lhs (const String&): [IN] The input string, typically a shell command line.
//
// [Returns]
//   first  (String): the lhs string with the last token stripped.
//   second (Path)  : the resolved directory path (falls back to ".").

bool is_text_file(const Path& target);
// Return true if the file is likely a text file (no null bytes in the first 1 KiB).
//
// [Args]
//   target (const Path&): [IN] The file path to check.
//
// [Returns]
//   (bool): True if the file is likely a text file, false otherwise.

String join(const Vector<String>& selected, const String& delim = " ");
// Join the selected items into a single string, separated by the specified delimiter.
//
// [Args]
//   selected (const Vector<String>&): [IN] The vector of selected items.
//   delim    (const String&)        : [IN] The delimiter to use for joining.
//
// [Returns]
//   (String): The joined string of selected items.

String pad_left(String s, size_t width);
// Pad the string on the left with spaces to ensure it has at least the given display width.
//
// [Args]
//   s     (String) : [IN] The input string to pad.
//   width (size_t) : [IN] The desired minimum display width.
//
// [Returns]
//   (String): The padded string, left-padded with spaces if necessary.

String pad_right(String s, size_t width);
// Pad the string on the right with spaces to ensure it has at least the given display width.
//
// [Args]
//   s     (String) : [IN] The input string to pad.
//   width (size_t) : [IN] The desired minimum display width.
//
// [Returns]
//   (String): The padded string, right-padded with spaces if necessary.

String read_file(const Path& path, int32_t max_bytes = -1);
// Read the entire contents of a file into a string.
//
// [Args]
//   path      (const Path&): [IN] The file path to read.
//   max_bytes (int)        : [IN] Maximum number of bytes to read (default: -1 for no limit).
//
// [Returns]
//   (String): The contents of the file as a string, or an empty string if the file cannot be read.

Generator<String> read_lines(const Path& path) noexcept;
// Read file contents in a generator manner.
//
// [Args]
//   path (const Path&): [IN] Target file path.
//
// [Yields]
//   (String): Line of the given file.

String shlex_join(const Vector<String>& args);
// Join a list of strings into a single shell-safe command string.
//
// [Args]
//   args (const Vector<String>&): [IN] The list of strings to join.
//
// [Returns]
//   (String): The shell-quoted command string.

String shlex_quote(StringView sv);
// Shell-quote a string so it is safe to use in a shell command.
//
// [Args]
//   sv (StringView): [IN] The input string.
//
// [Returns]
//   (String): The shell-quoted string.

Vector<StringView> shlex_split(StringView sv);
// Split a shell command string into tokens, respecting single/double quotes and backslash escapes.
//
// [Args]
//   sv (StringView): [IN] The shell command string to split.
//
// [Returns]
//   (Vector<StringView>): The list of token views extracted from the input string.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
