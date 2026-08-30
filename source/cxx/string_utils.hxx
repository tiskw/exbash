////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: string_utils.hxx                                                            ///
///                                                                                              ///
/// Utility functions for string manipulation, such as clipping and colorization.                ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef STRING_UTILS_HXX
#define STRING_UTILS_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Generator<StringView> chunk(StringView sv, uint16_t width);
// A helper function to chunk a given string into multiple string views of the specified width
// and return the chunked string views in generator manner.
//
// [Args]
//   sv    (StringView): [IN] Input string to be chunked.
//   width (uint16_t)  : [IN] Width to chunk the string into
//
// [Returns]
//   (Generator<StringView>): Generator of chunked string views.

StringView textclip(StringView sv, uint16_t width);
// A helper function to clip a given string to the specified width and return the clipped string.
//
// [Args]
//   sv    (StringView): [IN] Input string to be clipped.
//   width (uint16_t)  : [IN] Width to clip the string to
//
// [Returns]
//   (StringView): Clipped string view.

String colorize(StringView sv);
// A helper function to colorize a given string and return the colorized string.
//
// [Args]
//   sv (StringView): [IN] Input string to be colorized.
//
// [Returns]
//   (String): Colorized string.

String insert_cursor(StringView sv, uint16_t cursor_pos);
// A helper function to insert a pseudo cursor (reverse effect) at the specified position.
//
// [Args]
//   sv         (StringView): [IN] Input string to insert the cursor into.
//   cursor_pos (uint16_t)  : [IN] Position to insert the cursor at.
//
// [Returns]
//   (String): String with the cursor symbol inserted.

uint16_t width(StringView sv);
// A helper function to calculate the width of a given string.
//
// [Args]
//   sv (StringView): [IN] Input string to calculate the width of.
//
// [Returns]
//   (uint16_t): Width of the input string.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
