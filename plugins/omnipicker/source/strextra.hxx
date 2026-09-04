////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: strextra.hxx                                                                ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef STREXTRA_HXX
#define STREXTRA_HXX

// Include custom headers.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

String clipstr(StringView text, int32_t width);
// Clip a UTF-8 string so its display width does not exceed the given limit.
//
// [Args]
//   text  (const String&): [IN] The input string.
//   width (int32_t)      : [IN] The maximum display width.
//
// [Returns]
//   (String): The clipped string.

String fitstr(StringView text, int32_t width);
// Fit a UTF-8 string to the given display width and padding with spaces.
//
// [Args]
//   text  (const String&): [IN] The input string.
//   width (int32_t)      : [IN] The desired display width.
//
// [Returns]
//   (String): The fitted string.

bool is_ansi(StringView sv);
// Check if a string is an ANSI SGR escape sequence.
//
// [Args]
//   sv (StringView): [IN] The input string.
//
// [Returns]
//   (bool): True if the string is an ANSI SGR escape sequence, false otherwise.

StringView remove_last_utf8_char(StringView sv);
// Remove the last Unicode character from a UTF-8 string.
//
// [Args]
//   sv (StringView): [IN/OUT] The input string.
//
// [Returns]
//   (StringView): The string with the last Unicode character removed.

Generator<int32_t> parse_ansi(const String& code);
// Parse an ANSI SGR escape sequence and return the corresponding color code.
//
// [Args]
//   code (const String&): [IN] The ANSI SGR escape sequence.
//
// [Yields]
//   (int32_t): The color code corresponding to the ANSI escape sequence, or 0 if the code is empty.

Generator<String> split_ansi(const String& text);
// Split a string into segments, separating ANSI SGR escape sequences from normal text.
//
// [Args]
//   text (const String&): [IN] The input string.
//
// [Returns]
//   (Generator<String>): A generator yielding segments of the input string, where each segment is either an ANSI SGR escape sequence or a substring of normal text.

Generator<Tuple<int32_t, String>> split_lines(const String& str);
// Split a string into lines, handling both '\n' and '\r\n' line endings.
//
// [Args]
//   str (const String&): [IN] The input string to split into lines.
//
// [Yields]
//   (int32_t): The line index (starting from 0).
//   (String) : The line content as a string view.

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

int32_t strwidth(const String& text);
// Return the total display width of a string, ignoring ANSI SGR escape sequences.
//
// [Args]
//   text (const String&): [IN] The input string.
//
// [Returns]
//   (int): The total display width of the string.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
