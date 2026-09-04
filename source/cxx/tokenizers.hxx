////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: tokenizers.hxx                                                              ///
///                                                                                              ///
/// Functions to tokenize a string.                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TOKENIZERS_HXX
#define TOKENIZERS_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Data types
////////////////////////////////////////////////////////////////////////////////////////////////////

enum TokenizeOption : uint8_t
{
    TOKENIZE_PLAIN   = 0,       // Plain tokenization without any special options.
    TOKENIZE_KEEP_WS = 1 << 0,  // Keep white-space tokens in the result.
    TOKENIZE_DEQUOTE = 1 << 1,  // Strip single/double quotes from tokens.
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Generator<StringView> tokenize(StringView sv, TokenizeOption option = TOKENIZE_PLAIN);
// Split the given string to tokens.
//
// [Args]
//   sv     (StringView)    : [IN] Target string to be tokenized.
//   option (TokenizeOption): [IN] Tokenization options.
//
// [Returns]
//   (Generator<StringView>): Tokenized string views.

Generator<String> tokenize_with_placeholder_replacement(StringView sv, const StringMap& extra, TokenizeOption option = TOKENIZE_PLAIN);
// Tokenize the given command string with placeholder replacement.
//
// [Args]
//   sv               (const String&)   : [IN] Target string to be tokenize.
//   extra            (const StringMap&): [IN] Extra placeholder values.
//   keep_whitespaces (bool)            : [IN] If true, white-space tokens will be kept in the result.
//
// [Returns]
//   (Generator<String>): Tokenized string views with placeholder replacement.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
