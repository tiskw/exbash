////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: utils.hxx                                                                   ///
///                                                                                              ///
/// A collection of utility functions that can be used in various parts of the project.          ///
/// Some of these functions are implemented as templates to allow for flexibility in the types   ///
/// of arguments they can accept.                                                                ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef UTILS_HXX
#define UTILS_HXX

// Include STL headers.
#include <cstdint>
#include <future>

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility templates
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline constexpr T max(const T& a, const T& b)
{ return (a > b) ? a : b; }

template <typename T>
inline constexpr T min(const T& a, const T& b)
{ return (a < b) ? a : b; }

template <typename T>
inline constexpr T clip(const T& x, const T& low, const T& high)
{ return min(max(x, low), high); }

template <typename T>
void deduplicate(Vector<T>& vector) noexcept
// Deduplicate the given vector in an in-place manner.
//
// [Args]
//   vector (Vector<T>&): [IN] The target vector.
//
{   // {{{

    // Sort the vector.
    std::sort(vector.begin(), vector.end());

    // Remove duplicated command names.
    vector.erase(std::unique(vector.begin(), vector.end()), vector.end());

}   // }}}

template<typename F, typename... Args>
inline auto launch_async(F&& f, Args&&... args)
// Wrapper of std::async to launch a function asynchronously.
//
// [Args]
//   f    (F&&)  : [IN] The function to be launched.
//   args (Args&&...): [IN] Arguments to be passed to the function.
//
// [Returns]
//   Future object that holds the result of the function.
//
{   // {{{

    return std::async(std::launch::async, std::forward<F>(f), std::forward<Args>(args)...);

}   // }}}

template <typename T_in, typename T_out, typename F>
Vector<T_out> transform(const Vector<T_in>& xs, F&& func) noexcept
// Similar to std::transform, but returns a vector instance.
//
// [Args]
//   xs   (const Vector<T_in>&): [IN] Input vector.
//   func (F&&)                : [IN] Transform function.
//
// [Returns]
//   (Vector<T_out>): Output vector.
//
{   // {{{

    // Create array instance and reserve array size.
    Vector<T_out> ys(xs.size());

    // Apply transform.
    std::transform(xs.cbegin(), xs.cend(), ys.begin(), func);

    // Returns the result array.
    return ys;

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////

String expand_tilde(StringView path);
// Expand "~" to the home directory path.
//
// [Args]
//   path (StringView): [IN] Input path.
//
// [Returns]
//   (String): Output path.

Size get_terminal_size(void) noexcept;
// Get terminal size.
//
// [Returns]
//   (Size): Terminal size (cols and rows).

String get_time(time_t raw_time, const char* format) noexcept;
// Get time string.
//
// [Returns]
//   (String): Date time string.

Generator<String> readline(const char* path) noexcept;
// Read file contents in a generator manner.
//
// [Args]
//   path (const char*): [IN] Target file path.
//
// [Yields]
//   (String): Line of the given file.

String replace(StringView target, StringView oldstr, StringView newstr) noexcept;
// Replace string.
//
// [Args]
//   target (StringView): [IN] Target string to be replaced.
//   oldstr (StringView): [IN] This string will be replaced to `newstr`.
//   newstr (StringView): [IN] The `oldstr` will be replaced to this string.
//
// [Returns]
//   (String): Replaced string.

Generator<StringView> split(StringView str, StringView delim) noexcept;
// Split the given string with the given delimiter.
//
// [Args]
//   str   (StringView): [IN] Target string.
//   delim (StringView): [IN] Delimiter string.
//
// [Returns]
//   (Generator<StringView>): Split strings.

StringView strip(StringView sv, bool left = true, bool right = true) noexcept;
// Strip white-spaces from both front and end of the given string.
//
// [Args]
//   sv    (StringView): [IN] Target string to be stripped.
//   left  (bool)      : [IN] Stripped from the front if true.
//   right (bool)      : [IN] Stripped from the end if true.
//
// [Returns]
//   (StringView): Stripped string.

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

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
