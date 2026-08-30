////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: utf8.cxx                                                                    ///
///                                                                                              ///
/// This source code is written with reference to the utf8proc library, ver 2.11.3.              ///
/// <https://github.com/JuliaStrings/utf8proc>                                                   ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "utf8.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////////////////////////

#define utf_cont(ch)  (((ch) & 0xc0) == 0x80)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Data types
////////////////////////////////////////////////////////////////////////////////////////////////////

// Struct containing information about a codepoint.
struct utf8_property_t
{   // {{{

    // Unicode category.
    int16_t category;
    int16_t combining_class;

    // Bidirectional class.
    int16_t bidi_class;

    // Decomposition type.
     int16_t decomp_type;
    uint16_t decomp_seqindex;
    uint16_t casefold_seqindex;
    uint16_t uppercase_seqindex;
    uint16_t lowercase_seqindex;
    uint16_t titlecase_seqindex;

    // Character combining table.
    //
    // The character combining table is formally indexed by two characters, the first and second
    // character that might form a combining pair. The table entry then contains the combined
    // character. Most character pairs cannot be combined. There are about 1,000 characters that
    // can be the first character in a combining pair, and for most, there are only a handful for
    // possible second characters.
    //
    // The combining table is stored as sparse matrix in the CSR (compressed sparse row) format.
    // That is, it is stored as two arrays, `utf8proc_uint32_t utf8proc_combinations_second[]` and
    // `utf8proc_uint32_t utf8proc_combinations_combined[]`. These contain the second combining
    // characters and the combined character of every combining pair.
    //
    // - `comb_index`: Index into the combining table if this character is the first character
    //                 in a combining pair, else 0x3ff
    // - `comb_length`: Number of table entries for this first character
    // - `comb_is_second`: As optimization we also record whether this character is the second
    //                     combining character in any pair. If not, we can skip the table lookup.
    //
    // A table lookup starts from a given character pair. It first checks whether the first
    // character is stored in the table (checking whether the index is 0x3ff) and whether the second
    // index is stored in the table (looking at `comb_is_second`). If so, the `comb_length` table
    // entries will be checked sequentially for a match.
    uint16_t comb_index:10;
    uint16_t comb_length:5;
    uint16_t comb_issecond:1;
    unsigned bidi_mirrored:1;
    unsigned comp_exclusion:1;

    // Can this codepoint be ignored?
    // Used by utf8proc_decompose_char() when UTF8_IGNORE is passed as an option.
    unsigned ignorable:1;
    unsigned control_boundary:1;

    // The width of the codepoint.
    unsigned charwidth:2;

    // East Asian width class A.
    unsigned ambiguous_width:1;
    unsigned pad:1;

    // Boundclass.
    unsigned boundclass:6;
    unsigned indic_conjunct_break:2;

};  // }}}

// Unicode categories.
enum utf8_category_t
{   // {{{

    UTF8_CATEGORY_CN =  0, // Other, not assigned
    UTF8_CATEGORY_LU =  1, // Letter, uppercase
    UTF8_CATEGORY_LL =  2, // Letter, lowercase
    UTF8_CATEGORY_LT =  3, // Letter, titlecase
    UTF8_CATEGORY_LM =  4, // Letter, modifier
    UTF8_CATEGORY_LO =  5, // Letter, other
    UTF8_CATEGORY_MN =  6, // Mark, nonspacing
    UTF8_CATEGORY_MC =  7, // Mark, spacing combining
    UTF8_CATEGORY_ME =  8, // Mark, enclosing
    UTF8_CATEGORY_ND =  9, // Number, decimal digit
    UTF8_CATEGORY_NL = 10, // Number, letter
    UTF8_CATEGORY_NO = 11, // Number, other
    UTF8_CATEGORY_PC = 12, // Punctuation, connector
    UTF8_CATEGORY_PD = 13, // Punctuation, dash
    UTF8_CATEGORY_PS = 14, // Punctuation, open
    UTF8_CATEGORY_PE = 15, // Punctuation, close
    UTF8_CATEGORY_PI = 16, // Punctuation, initial quote
    UTF8_CATEGORY_PF = 17, // Punctuation, final quote
    UTF8_CATEGORY_PO = 18, // Punctuation, other
    UTF8_CATEGORY_SM = 19, // Symbol, math
    UTF8_CATEGORY_SC = 20, // Symbol, currency
    UTF8_CATEGORY_SK = 21, // Symbol, modifier
    UTF8_CATEGORY_SO = 22, // Symbol, other
    UTF8_CATEGORY_ZS = 23, // Separator, space
    UTF8_CATEGORY_ZL = 24, // Separator, line
    UTF8_CATEGORY_ZP = 25, // Separator, paragraph
    UTF8_CATEGORY_CC = 26, // Other, control
    UTF8_CATEGORY_CF = 27, // Other, format
    UTF8_CATEGORY_CS = 28, // Other, surrogate
    UTF8_CATEGORY_CO = 29, // Other, private use

};  // }}}

// Bidirectional character classes.
enum utf8_bidi_class_t
{   // {{{

    UTF8_BIDI_CLASS_L     = 1, // Left-to-Right
    UTF8_BIDI_CLASS_LRE   = 2, // Left-to-Right Embedding
    UTF8_BIDI_CLASS_LRO   = 3, // Left-to-Right Override
    UTF8_BIDI_CLASS_R     = 4, // Right-to-Left
    UTF8_BIDI_CLASS_AL    = 5, // Right-to-Left Arabic */
    UTF8_BIDI_CLASS_RLE   = 6, // Right-to-Left Embedding */
    UTF8_BIDI_CLASS_RLO   = 7, // Right-to-Left Override */
    UTF8_BIDI_CLASS_PDF   = 8, // Pop Directional Format */
    UTF8_BIDI_CLASS_EN    = 9, // European Number */
    UTF8_BIDI_CLASS_ES   = 10, // European Separator */
    UTF8_BIDI_CLASS_ET   = 11, // European Number Terminator */
    UTF8_BIDI_CLASS_AN   = 12, // Arabic Number */
    UTF8_BIDI_CLASS_CS   = 13, // Common Number Separator */
    UTF8_BIDI_CLASS_NSM  = 14, // Nonspacing Mark */
    UTF8_BIDI_CLASS_BN   = 15, // Boundary Neutral */
    UTF8_BIDI_CLASS_B    = 16, // Paragraph Separator */
    UTF8_BIDI_CLASS_S    = 17, // Segment Separator */
    UTF8_BIDI_CLASS_WS   = 18, // Whitespace */
    UTF8_BIDI_CLASS_ON   = 19, // Other Neutrals */
    UTF8_BIDI_CLASS_LRI  = 20, // Left-to-Right Isolate */
    UTF8_BIDI_CLASS_RLI  = 21, // Right-to-Left Isolate */
    UTF8_BIDI_CLASS_FSI  = 22, // First Strong Isolate */
    UTF8_BIDI_CLASS_PDI  = 23, // Pop Directional Isolate */

};  // }}}

// Decomposition type.
enum utf8_decomp_type_t
{   // {{{

    UTF8_DECOMP_TYPE_FONT      = 1, // Font
    UTF8_DECOMP_TYPE_NOBREAK   = 2, // Nobreak
    UTF8_DECOMP_TYPE_INITIAL   = 3, // Initial
    UTF8_DECOMP_TYPE_MEDIAL    = 4, // Medial
    UTF8_DECOMP_TYPE_FINAL     = 5, // Final
    UTF8_DECOMP_TYPE_ISOLATED  = 6, // Isolated
    UTF8_DECOMP_TYPE_CIRCLE    = 7, // Circle
    UTF8_DECOMP_TYPE_SUPER     = 8, // Super
    UTF8_DECOMP_TYPE_SUB       = 9, // Sub
    UTF8_DECOMP_TYPE_VERTICAL = 10, // Vertical
    UTF8_DECOMP_TYPE_WIDE     = 11, // Wide
    UTF8_DECOMP_TYPE_NARROW   = 12, // Narrow
    UTF8_DECOMP_TYPE_SMALL    = 13, // Small
    UTF8_DECOMP_TYPE_SQUARE   = 14, // Square
    UTF8_DECOMP_TYPE_FRACTION = 15, // Fraction
    UTF8_DECOMP_TYPE_COMPAT   = 16, // Compat

};  // }}}

// Boundclass property.
enum utf8_boundclass_t
{   // {{{

    UTF8_BOUNDCLASS_START              =  0,    // Start
    UTF8_BOUNDCLASS_OTHER              =  1,    // Other
    UTF8_BOUNDCLASS_CR                 =  2,    // Cr
    UTF8_BOUNDCLASS_LF                 =  3,    // Lf
    UTF8_BOUNDCLASS_CONTROL            =  4,    // Control
    UTF8_BOUNDCLASS_EXTEND             =  5,    // Extend
    UTF8_BOUNDCLASS_L                  =  6,    // L
    UTF8_BOUNDCLASS_V                  =  7,    // V
    UTF8_BOUNDCLASS_T                  =  8,    // T
    UTF8_BOUNDCLASS_LV                 =  9,    // Lv
    UTF8_BOUNDCLASS_LVT                = 10,    // Lvt
    UTF8_BOUNDCLASS_REGIONAL_INDICATOR = 11,    // Regional indicator
    UTF8_BOUNDCLASS_SPACINGMARK        = 12,    // Spacingmark
    UTF8_BOUNDCLASS_PREPEND            = 13,    // Prepend
    UTF8_BOUNDCLASS_ZWJ                = 14,    // Zero Width Joiner

    // The following are no longer used in Unicode 11, but we keep
    // the constants here for backward compatibility.
    UTF8_BOUNDCLASS_E_BASE             = 15,    // Emoji Base
    UTF8_BOUNDCLASS_E_MODIFIER         = 16,    // Emoji Modifier
    UTF8_BOUNDCLASS_GLUE_AFTER_ZWJ     = 17,    // Glue_After_ZWJ
    UTF8_BOUNDCLASS_E_BASE_GAZ         = 18,    // E_BASE + GLUE_AFTER_ZJW

    // The EXTENDED_PICTOGRAPHIC property is used in the Unicode 11 grapheme-boundary rules,
    // so we store it in the boundclass field.
    UTF8_BOUNDCLASS_EXTENDED_PICTOGRAPHIC = 19,
    UTF8_BOUNDCLASS_E_ZWG                 = 20, // UTF8_BOUNDCLASS_EXTENDED_PICTOGRAPHIC + ZWJ */

};  // }}}

// Indic_Conjunct_Break property.
enum utf8proc_indic_conjunct_break_t
{   // {{{

    UTF8_INDIC_CONJUNCT_BREAK_NONE      = 0,
    UTF8_INDIC_CONJUNCT_BREAK_LINKER    = 1,
    UTF8_INDIC_CONJUNCT_BREAK_CONSONANT = 2,
    UTF8_INDIC_CONJUNCT_BREAK_EXTEND    = 3,

};  // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Data tables
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "utf8data.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local static functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace {

    const utf8_property_t* utf8proc_get_property(int32_t uc)
    // Look up the properties for a given codepoint.
    //
    // [Args]
    //   uc (int32_t): [IN] Unicode codepoint.
    //
    // [Returns]
    //   (const utf8_property_t*): Pointer to the property struct of the codepoint.
    //
    {   // {{{

        // Safe-guard: Check the range of codepoint.
        if ((uc < 0) || (uc >= 0x110000))
            return utf8_properties;

        // Otherwise, look up the property table.
        return utf8_properties + (utf8_stage2table[
            utf8_stage1table[uc >> 8] + (uc & 0xFF)
        ]);

    }   // }}}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t utf8_byte_size(const uint8_t first_byte) noexcept
{   // {{{

    if ((first_byte & 0x80) == 0x00) return 1;  // 0xxxxxxx (00-7F)
    if ((first_byte & 0xE0) == 0xC0) return 2;  // 110xxxxx (C2-DF)
    if ((first_byte & 0xF0) == 0xE0) return 3;  // 1110xxxx (E0-EF)
    if ((first_byte & 0xF8) == 0xF0) return 4;  // 11110xxx (F0-F7)
    else                             return 0;  // Invalid UTF-8 leading byte.

}   // }}}

ptrdiff_t utf8_decode(const uint8_t *str, ptrdiff_t size, int32_t *dst)
{   // {{{

    // Initialize the output value.
    *dst = -1;

    // Do nothing for empty input string.
    if ((size == 0) or (str == nullptr) or (*str == '\0'))
        return 0;

    // Compute the maximum end of the input string to read.
    const uint8_t* end = str + ((size < 0) ? 4 : size);

    // Read the first byte.
    int32_t uc = *str++;

    // Case 1: Single-byte UTF-8.
    if (uc < 0x80)
    {
        *dst = uc;
        return 1;
    }

    // Must be between 0xc2 and 0xf4 inclusive to be valid UTF-8.
    if (static_cast<uint32_t>(uc - 0xc2) > (0xf4 - 0xc2))
        return UTF8_ERROR_INVALIDUTF8;

    // Case 2: 2-byte UTF-8.
    if (uc < 0xe0)
    {
       // Must have valid continuation character.
       if ((str >= end) or (not utf_cont(*str)))
           return UTF8_ERROR_INVALIDUTF8;

       *dst = ((uc & 0x1f) << 6) | (*str & 0x3f);

       return 2;
    }

    // Case 3: 3-byte UTF-8.
    else if (uc < 0xf0)
    {
       // Must have valid continuation character.
       if (((str + 1) >= end) or (not utf_cont(*str)) or (not utf_cont(str[1])))
          return UTF8_ERROR_INVALIDUTF8;

       // Check for surrogate chars.
       if ((uc == 0xed) and (*str > 0x9f))
           return UTF8_ERROR_INVALIDUTF8;

       // Decode the bytes to the UTF8 codepoint.
       uc = ((uc & 0xf) << 12) | ((*str & 0x3f) << 6) | (str[1] & 0x3f);

       if (uc < 0x800)
           return UTF8_ERROR_INVALIDUTF8;

       *dst = uc;

       return 3;
    }

    // Case 4: 4-byte UTF-8.
    else
    {
        // Must have 3 valid continuation characters
        if (((str + 2) >= end) or (not utf_cont(*str)) or (not utf_cont(str[1])) or (not utf_cont(str[2])))
           return UTF8_ERROR_INVALIDUTF8;

        // Make sure in correct range (0x10000 - 0x10ffff)
        if (uc == 0xf0)
        {
            if (*str < 0x90)
                return UTF8_ERROR_INVALIDUTF8;
        }
        else if (uc == 0xf4)
        {
            if (*str > 0x8f)
                return UTF8_ERROR_INVALIDUTF8;
        }

        // Decode the bytes to the UTF8 codepoint.
        *dst = ((uc & 7)<<18) | ((*str & 0x3f)<<12) | ((str[1] & 0x3f)<<6) | (str[2] & 0x3f);

        return 4;
    }

}   // }}}

ptrdiff_t utf8_encode(int32_t uc, uint8_t *dst)
{   // {{{

    // Case 1: Invalid codepoint.
    if (uc < 0x00)
    {
        return 0;
    }

    // Case 2: 1 byte UTF-8.
    else if (uc < 0x80)
    {
        dst[0] = static_cast<uint8_t>(uc);
        return 1;
    }

    // Case 3: 2 bytes UTF-8.
    //
    // Note: We allow encoding 0xd800-0xdfff here, so as not to change the API,
    //       however, these are actually invalid in UTF-8.
    else if (uc < 0x800)
    {
        dst[0] = static_cast<uint8_t>(0xC0 + (uc >> 6));
        dst[1] = static_cast<uint8_t>(0x80 + (uc & 0x3F));
        return 2;
    }

    // Case 4: 3 bytes UTF-8.
    else if (uc < 0x10000)
    {
        dst[0] = static_cast<uint8_t>(0xE0 + ( uc >> 12));
        dst[1] = static_cast<uint8_t>(0x80 + ((uc >>  6) & 0x3F));
        dst[2] = static_cast<uint8_t>(0x80 + ( uc        & 0x3F));
        return 3;
    }

    // Case 5: 4 bytes UTF-8.
    else if (uc < 0x110000)
    {
        dst[0] = static_cast<uint8_t>(0xF0 + ( uc >> 18));
        dst[1] = static_cast<uint8_t>(0x80 + ((uc >> 12) & 0x3F));
        dst[2] = static_cast<uint8_t>(0x80 + ((uc >>  6) & 0x3F));
        dst[3] = static_cast<uint8_t>(0x80 + ( uc        & 0x3F));
        return 4;
    }

    // Case 6: Invalid codepoint.
    else
    {
        return 0;
    }

}   // }}}

int16_t utf8_width(int32_t c)
{   // {{{

    return utf8proc_get_property(c)->charwidth;

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public utility wrapper functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Generator<StringView> utf8_iter(const char* str, size_t size)
{   // {{{

    // Compute the size of the input string if it is not given.
    if (size == 0)
        size = std::strlen(str);

    // Compute the end pointer of the input string.
    const char* str_end = str + size;

    // A variable to store the byte size of the current UTF-8 character.
    uint8_t byte_size;

    while (str < str_end)
    {

        // Case 1: ANSI escape sequence.
        if ((*str == '\x1B') and ((str + 1) < str_end) and (str[1] == '['))
        {
            // Skip 2 bytes because the first two bytes are always "\x1B[".
            byte_size = 2;

            // Skip the rest of the ANSI escape sequence.
            while (((str + byte_size) < str_end) and (*(str + byte_size) != '\0') and not std::isalpha(*(str + byte_size)))
                ++byte_size;
            ++byte_size;
        }

        // Case 2: Regular UTF-8 character.
        else
        {
            byte_size = utf8_byte_size(static_cast<uint8_t>(*str));
        }

        // Returns the codepoint as a string view.
        co_yield StringView(str, (byte_size > 0) ? static_cast<size_t>(byte_size) : 1);

        // Update the string pointer.
        str += (byte_size > 0) ? byte_size : 1;
    };

}   // }}}

Generator<Tuple<int32_t, const char*>> utf8_decode_iter(const char* str, size_t size)
{   // {{{

    // Compute the size of the input string if it is not given.
    if (size == 0)
        size = std::strlen(str);

    // Compute the end pointer of the input string.
    const char* str_end = str + size;

    while (str < str_end)
    {
        int32_t codepoint  = 0;
        PtrDiff read_bytes = 0;

        if ((*str == '\x1B') and ((str + 1) < str_end) and (str[1] == '['))
        {
            // Skip the ANSI escape sequence.
            while (((str + read_bytes) < str_end) and (*(str + read_bytes) != '\0') and not std::isalpha(*(str + read_bytes)))
                ++read_bytes;
            ++read_bytes;

            // Set the codepoint to -1 (invalid UTF-8) to indicate that this is an ANSI escape sequence.
            codepoint = -1;
        }
        else
        {
            // Decode a single character.
            read_bytes = utf8_decode(reinterpret_cast<const uint8_t*>(str), -1, &codepoint);
        }

        // Returns the codepoint.
        co_yield {codepoint, str};

        // Update the string pointer.
        str += (read_bytes > 0) ? read_bytes : 1;
    };

}   // }}}

CharX utf8_decode_next_charx(const char* str)
{   // {{{

    // Do nothing for empty input string.
    if (str == nullptr)
        return CharX(nullptr, 0);

    // Get the byte size of the UTF-8 character based on the first byte.
    uint8_t byte_size = utf8_byte_size(static_cast<uint8_t>(*str));

    // Return the character as CharX.
    return CharX(str, byte_size);

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
