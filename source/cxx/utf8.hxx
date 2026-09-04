////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: utf8.hxx                                                                    ///
///                                                                                              ///
/// Functions for manipulating UTF-8 encoded strings.                                            ///
/// This source code is written with reference to the utf8proc library, ver 2.11.3.              ///
/// <https://github.com/JuliaStrings/utf8proc>                                                   ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef UTF8_HXX
#define UTF8_HXX

// Include STL headers.
#include <cstdint>
#include <cstddef>

// Include the headers of custom modules.
#include "char_x.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

#define UTF8_ERROR_NOMEM       (-1)
#define UTF8_ERROR_OVERFLOW    (-2)
#define UTF8_ERROR_INVALIDUTF8 (-3)
#define UTF8_ERROR_NOTASSIGNED (-4)
#define UTF8_ERROR_INVALIDOPTS (-5)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t utf8_byte_size(const uint8_t first_byte) noexcept;
// Return the number of bytes in a UTF-8 character based on the first byte.
//
// [Args]
//   first_byte (uint8_t): [IN] The first byte of the UTF-8 character.
//
// [Returns]
//   (uint8_t): The number of bytes in the UTF-8 character, or 0 if the first byte is not a valid UTF-8 leading byte.

ptrdiff_t utf8_decode(const uint8_t* str, ptrdiff_t strlen, int32_t* dst);
// Parse a single UTF-8 character from the given string.
//
// [Args]
//   str    (const uint8_t*): [IN]  Pointer to the input string.
//   strlen (ptrdiff_t)     : [IN]  Length of the string. If negative, up to 4 bytes are read.
//   dst    (int32_t* )     : [OUT] Pointer to the variable to store the parsed codepoint.
//
// [Returns]
//   (ptrdiff_t): Number of bytes read if success, and negative error code otherwise.

ptrdiff_t utf8_encode(int32_t uc, uint8_t* dst);
// Encode a single Unicode codepoint to a UTF-8 string.
//
// [Args]
//   uc  (uint32_t)  : [IN]  Unicode codepoint.
//   dst (uint8_t*)  : [OUT] Pointer to the output string buffer (must be at least 4 bytes long).
//
// [Returns]
//   (ptrdiff_t): Number of bytes written if success, and 0 otherwise.

int16_t utf8_width(int32_t c);
// Return a character width, similar to "wcwidth"
// (but except portable and hopefully less buggy!).
//
// [Args]
//   c (int32_t): [IN] Unicode codepoint.
//
// [Returns]
//   (int16_t): Character width.

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public utility wrapper functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Generator<StringView> utf8_iter(const char* str, size_t size = 0);
// Iteratively parse UTF-8 characters from the given string and return them as StringView.
//
// [Args]
//   str  (const char*): [IN] Pointer to the input string.
//   size (size_t)     : [IN] Length of the input string. If zero, the length is computed using std::strlen.
//
// [Returns]
//   (Generator<StringView>): Generator of StringView of UTF-8 characters.

Generator<Tuple<int32_t, const char*>> utf8_decode_iter(const char* str, size_t size = 0);
// Iteratively parse UTF-8 characters from the given string.
//
// [Args]
//
// [Returns]
//   (Generator<Tuple<int32_t, uint8_t*>>): Codepoint of UTF-8 character.

CharX utf8_decode_next_charx(const char* str);
// Iteratively parse UTF-8 characters from the given string and return them as CharX.
//
// [Args]
//   str (const char*): [IN] Pointer to the input string.
//
// [Returns]
//   (CharX): The next UTF-8 character in the input string as CharX.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
