////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: text_editor_emacs.cxx                                                       ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "text_editor_emacs.hxx"

// Include the headers of custom modules.
#include "utf8.hxx"

// Include the standard library headers.
#include <cctype>

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    // Bring TextEditor's protected types and utilities into scope so that the word-motion
    // helper functions below can use them without qualification.
    using CharClass = TextEditor::CharClass;
    using CharInfo  = TextEditor::CharInfo;

    PtrDiff word_fwd_emacs(StringView rhs)
    // Return the number of UTF-8 characters to advance for M-f (Emacs forward-word).
    // Skips non-WORD characters, then skips WORD characters.
    //
    {   // {{{

        // If the input string is empty, there are no characters to skip, so return 0.
        if (rhs.empty()) return 0;

        // Get the character info vector for rhs, and if it's empty, return 0.
        const Vector<CharInfo> chars = TextEditor::collect_chars(rhs);
        if (chars.empty()) return 0;

        SizeType idx   = 0;
        PtrDiff  count = 0;

        while ((idx < chars.size()) and (chars[idx].cls != CharClass::WORD)) { ++idx; ++count; }
        while ((idx < chars.size()) and (chars[idx].cls == CharClass::WORD)) { ++idx; ++count; }

        return count;

    }   // }}}

    PtrDiff word_bwd_emacs(StringView lhs)
    // Return the number of UTF-8 characters to retreat for M-b (Emacs backward-word).
    // Skips non-WORD characters backward, then skips WORD characters backward.
    //
    {   // {{{

        // If the input string is empty, there are no characters to skip, so return 0.
        if (lhs.empty()) return 0;

        // Get the character info vector for lhs, and if it's empty, return 0.
        const Vector<CharInfo> chars = TextEditor::collect_chars(lhs);
        if (chars.empty()) return 0;

        int32_t idx   = static_cast<int32_t>(chars.size()) - 1;
        PtrDiff count = 0;

        while ((idx >= 0) and (chars[idx].cls != CharClass::WORD)) { --idx; ++count; }
        while ((idx >= 0) and (chars[idx].cls == CharClass::WORD)) { --idx; ++count; }

        return count;

    }   // }}}

    PtrDiff kill_to_space_bwd(StringView lhs)
    // Return the number of UTF-8 characters to delete backward for C-w (unix-word-rubout).
    // Kills backward through non-SPACE chars, then through SPACE chars.
    //
    {   // {{{

        // If the input string is empty, there are no characters to skip, so return 0.
        if (lhs.empty()) return 0;

        // Get the character info vector for rhs, and if it's empty, return 0.
        const Vector<CharInfo> chars = TextEditor::collect_chars(lhs);
        if (chars.empty()) return 0;

        int32_t idx   = static_cast<int32_t>(chars.size()) - 1;
        PtrDiff count = 0;

        while ((idx >= 0) and (chars[idx].cls != CharClass::SPACE)) { --idx; ++count; }
        while ((idx >= 0) and (chars[idx].cls == CharClass::SPACE)) { --idx; ++count; }

        return count;

    }   // }}}

    String transform_word_case(StringView sv, int wcase)
    // Return a copy of sv with case transformation applied to ASCII letters.
    // Multi-byte UTF-8 characters are passed through unchanged.
    //
    // [Args]
    //   sv    (StringView): [IN] The word to transform.
    //   wcase (int)       : [IN] 0 = UPPER, 1 = lower, 2 = Capitalize.
    //
    // [Returns]
    //   (String): Transformed word.
    //
    {   // {{{

        String result;
        result.reserve(sv.size());
        bool first = true;

        for (SizeType i = 0; i < sv.size(); )
        {
            uint8_t c = static_cast<uint8_t>(sv[i]);

            if (c < 0x80)
            {
                char ch;
                if      (wcase == 0) ch = static_cast<char>(std::toupper(c));
                else if (wcase == 1) ch = static_cast<char>(std::tolower(c));
                else                 ch = first ? static_cast<char>(std::toupper(c))
                                                : static_cast<char>(std::tolower(c));
                result += ch;
                first = false;
                ++i;
            }
            else
            {
                // Multi-byte UTF-8: pass through without transformation.
                uint8_t bsz = utf8_byte_size(c);
                SizeType n  = (bsz > 0 && i + bsz <= sv.size()) ? bsz : 1;
                result.append(sv.data() + i, n);
                first = false;
                i += n;
            }
        }
        return result;

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditorEmacs: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

TextEditorEmacs::TextEditorEmacs(StringView lhs, StringView rhs, const Deque<String>& hists) : TextEditor(lhs, rhs, hists), meta_pending(false)
{ /* Do nothing, initializer lists only. */ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditorEmacs: Edit functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void TextEditorEmacs::edit(StringView sv)
{   // {{{

    // Ignore empty input.
    if (sv.empty()) return;

    this->process(sv.data(), sv.size());

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditorEmacs: Private functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void TextEditorEmacs::process(const char* str, SizeType size)
{   // {{{

    GapBuffer& buffer = this->current_buffer();

    // If meta_pending is set, the previous input was ESC: interpret this char as M-<char>.
    if (this->meta_pending)
    {
        this->meta_pending = false;
        if (size == 1) { this->process_meta(str[0]); return; }
        // Multi-byte key after ESC (e.g. arrow key): fall through to normal processing.
    }

    // Single-byte control characters.
    if (size == 1)
    {
        const char c = str[0];

        switch (c)
        {
            // C-a: move to beginning of line.
            case '\x01':
                buffer.move_top();
                return;

            // C-b: move backward one character.
            case '\x02':
                buffer.move_cursor(-1);
                return;

            // C-d: delete character forward.
            case '\x04':
                if (not buffer.rhs_view().empty()) buffer.deletekey(1);
                return;

            // C-e: move to end of line.
            case '\x05':
                buffer.move_end();
                return;

            // C-f: move forward one character.
            case '\x06':
                buffer.move_cursor(1);
                return;

            // C-h (BS): delete character backward.
            case '\x08':
                if (not buffer.lhs_view().empty()) buffer.backspace(1);
                return;

            // C-k: kill to end of line.
            case '\x0B':
                if (not buffer.rhs_view().empty())
                {
                    this->kill_ring = String(buffer.rhs_view());
                    buffer.erase_rhs();
                }
                return;

            // C-n: next history (same as Down arrow).
            case '\x0E':
                this->change_buffer(1);
                return;

            // C-p: previous history (same as Up arrow).
            case '\x10':
                this->change_buffer(-1);
                return;

            // C-t: transpose characters.
            case '\x14':
                this->transpose_chars();
                return;

            // C-u: kill to beginning of line.
            case '\x15':
                if (not buffer.lhs_view().empty())
                {
                    this->kill_ring = String(buffer.lhs_view());
                    buffer.erase_lhs();
                }
                return;

            // C-w: kill backward word (whitespace-delimited; unix-word-rubout).
            case '\x17':
            {
                const PtrDiff n = kill_to_space_bwd(buffer.lhs_view());
                if (n > 0)
                {
                    this->kill_ring = extract_back(buffer.lhs_view(), n);
                    buffer.backspace(n);
                }
                return;
            }

            // C-y: yank (paste kill ring at cursor).
            case '\x19':
                if (not this->kill_ring.empty()) buffer.insert(this->kill_ring);
                return;

            // ESC: set meta_pending for the next key.
            case '\x1B':
                this->meta_pending = true;
                return;

            // DEL (0x7F): delete character backward.
            case '\x7F':
                if (not buffer.lhs_view().empty()) buffer.backspace(1);
                return;

            default:
                // Ignore remaining control characters (C-c, C-g, C-l, etc.).
                if (static_cast<unsigned char>(c) < 0x20) return;
                break;
        }
    }

    // Arrow key sequences: \x1B[ A/B/C/D.
    if ((size == 3) and (str[0] == '\x1B') and (str[1] == '['))
    {
        switch (str[2])
        {
            case 'A': this->change_buffer(-1);  return;  // Up
            case 'B': this->change_buffer(+1);  return;  // Down
            case 'C': buffer.move_cursor(+1);   return;  // Right
            case 'D': buffer.move_cursor(-1);   return;  // Left
            default: return;
        }
    }

    // Regular printable character (including multi-byte UTF-8): insert at cursor.
    buffer.insert(StringView(str, size));

}   // }}}

void TextEditorEmacs::process_meta(char ch)
{   // {{{

    GapBuffer& buffer = this->current_buffer();

    switch (ch)
    {
        // M-b: move backward one word.
        case 'b':
        {
            const PtrDiff n = word_bwd_emacs(buffer.lhs_view());
            if (n > 0) buffer.move_cursor(-n);
            break;
        }

        // M-c: capitalize word.
        case 'c':
            this->word_case_transform(2);
            break;

        // M-d: kill word forward.
        case 'd':
        {
            const PtrDiff n = word_fwd_emacs(buffer.rhs_view());
            if (n > 0)
            {
                this->kill_ring = extract_front(buffer.rhs_view(), n);
                buffer.deletekey(n);
            }
            break;
        }

        // M-f: move forward one word.
        case 'f':
        {
            const PtrDiff n = word_fwd_emacs(buffer.rhs_view());
            if (n > 0) buffer.move_cursor(n);
            break;
        }

        // M-l: lowercase word.
        case 'l':
            this->word_case_transform(1);
            break;

        // M-u: uppercase word.
        case 'u':
            this->word_case_transform(0);
            break;

        // M-DEL (0x7F): kill word backward.
        case '\x7F':
        {
            const PtrDiff n = word_bwd_emacs(buffer.lhs_view());
            if (n > 0)
            {
                this->kill_ring = extract_back(buffer.lhs_view(), n);
                buffer.backspace(n);
            }
            break;
        }

        // M-ESC: cancel meta (double ESC = no-op).
        case '\x1B':
            break;

        // Unrecognized meta key: insert the character as-is.
        default:
            if (static_cast<unsigned char>(ch) >= 0x20) buffer.insert(StringView(&ch, 1));
            break;
    }

}   // }}}

void TextEditorEmacs::transpose_chars(void)
{   // {{{

    GapBuffer& buffer = this->current_buffer();

    // Need at least 2 characters total to transpose.
    if (buffer.count() < 2) return;

    // If at end of line, step back one character first (so we swap the last two).
    if (buffer.rhs_view().empty()) buffer.move_cursor(-1);

    // Need a character before the cursor.
    if (buffer.lhs_view().empty()) return;

    // Find the start of the last UTF-8 character in lhs (the char before cursor).
    StringView lhs          = buffer.lhs_view();
    SizeType   before_start = lhs.size() - 1;
    while (before_start > 0 && (static_cast<uint8_t>(lhs[before_start]) & 0xC0) == 0x80)
        --before_start;
    String char_before(lhs.data() + before_start, lhs.size() - before_start);

    // First UTF-8 character of rhs (the char under cursor).
    StringView rhs        = buffer.rhs_view();
    uint8_t    bsz        = utf8_byte_size(static_cast<uint8_t>(rhs[0]));
    SizeType   under_size = (bsz > 0 && bsz <= rhs.size()) ? bsz : 1;
    String char_under(rhs.data(), under_size);

    // Remove both characters, then re-insert in swapped order.
    buffer.backspace(1);
    buffer.deletekey(1);
    buffer.insert(char_under);
    buffer.insert(char_before);
    // Cursor now sits after both characters.

}   // }}}

void TextEditorEmacs::word_case_transform(int wcase)
{   // {{{

    GapBuffer& buffer = this->current_buffer();
    StringView rhs    = buffer.rhs_view();
    if (rhs.empty()) return;

    const auto chars = collect_chars(rhs);
    SizeType   idx   = 0;
    PtrDiff    skip  = 0;

    // Skip any non-WORD prefix (spaces and punctuation) without modifying them.
    while (idx < chars.size() && chars[idx].cls != CharClass::WORD) { ++idx; ++skip; }

    PtrDiff word_len = 0;
    while (idx < chars.size() && chars[idx].cls == CharClass::WORD) { ++idx; ++word_len; }

    // Move past the non-word prefix.
    if (skip > 0) buffer.move_cursor(skip);

    if (word_len == 0) return;

    // Extract, transform, and replace the word.
    String word        = extract_front(buffer.rhs_view(), word_len);
    String transformed = transform_word_case(word, wcase);

    buffer.deletekey(word_len);
    buffer.insert(transformed);
    // Cursor is now positioned after the end of the transformed word.

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
