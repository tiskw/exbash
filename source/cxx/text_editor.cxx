////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: text_editor.cxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "text_editor.hxx"

// Include the standard library headers.
#include <cctype>

// Include the headers of custom modules.
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
    TextEditor::CharClass classify_char(const char* p) noexcept
    // Classify the UTF-8 character at p into SPACE / WORD / OTHER.
    // Used only by TextEditor::collect_chars.
    //
    // [Args]
    //   p (const char*): [IN] Pointer to the start of the UTF character to classify.
    //
    // [Returns]
    //   (TextEditor::CharClass): Character class of the character at p.
    //
    {   // {{{

        uint8_t c = static_cast<uint8_t>(*p);

        if ((c == ' ') or (c == '\t'))                   { return TextEditor::CharClass::SPACE; }
        if ((c == '_') or (c >= 0x80))                   { return TextEditor::CharClass::WORD;  }
        if (std::isalnum(static_cast<unsigned char>(c))) { return TextEditor::CharClass::WORD;  }
        else                                             { return TextEditor::CharClass::OTHER; }

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditor: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

TextEditor::TextEditor(StringView lhs, StringView rhs, const Deque<String>& hists)
    : buffer(lhs, rhs), hists_ref(hists), index(hists.size()), mode(Mode::INSERT)
{ /* Do nothing, initializer list only. */ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditor: Protected static utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<TextEditor::CharInfo> TextEditor::collect_char_info(StringView sv)
{   // {{{

    // Initialize the output vector.
    Vector<TextEditor::CharInfo> chars;

    // Initialize the position index.
    SizeType pos = 0;

    while (pos < sv.size())
    {
        // Record the byte position and character class of the current character.
        chars.push_back({pos, classify_char(sv.data() + pos)});

        // Advance the index by the byte size of the current UTF-8 character.
        uint8_t delta = utf8_byte_size(static_cast<uint8_t>(sv[pos]));
        pos += ((delta > 0) and (pos + delta <= sv.size())) ? delta : 1;
    }

    return chars;

}   // }}}

String TextEditor::extract_front(StringView sv, PtrDiff n)
{   // {{{

    // Do nothing if n is non-positive or sv is empty.
    if ((n <= 0) or sv.empty()) return "";

    // Initialize the position index and character count.
    SizeType pos   = 0;
    PtrDiff  count = 0;

    while ((pos < sv.size()) and (count < n))
    {
        // Advance the position index by the byte size of the current UTF-8 character.
        uint8_t bsz = utf8_byte_size(static_cast<uint8_t>(sv[pos]));
        pos += (bsz > 0 && pos + bsz <= sv.size()) ? bsz : 1;

        // Increment the character count.
        ++count;
    }

    return String(sv.data(), pos);

}   // }}}

String TextEditor::extract_back(StringView sv, PtrDiff n)
{   // {{{

    // Do nothing if n is non-positive or sv is empty.
    if ((n <= 0) or sv.empty()) return "";

    // Collect the character information for sv.
    const Vector<TextEditor::CharInfo> chars = collect_char_info(sv);

    // Return the entire string if n exceeds the number of characters in sv.
    if (n >= static_cast<PtrDiff>(chars.size()))
        return String(sv);

    // Otherwise, return the last n characters of sv.
    SizeType start = chars[chars.size() - n].byte_pos;
    return String(sv.data() + start, sv.size() - start);

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditor: Getter and setter functions
////////////////////////////////////////////////////////////////////////////////////////////////////

StringView TextEditor::get_lhs(void) const
{ return this->current_buffer().lhs_view(); }

StringView TextEditor::get_rhs(void) const
{ return this->current_buffer().rhs_view(); }

TextEditor::Mode TextEditor::get_mode(void) const noexcept
{ return this->mode; }

void TextEditor::set(StringView lhs, StringView rhs)
{ this->current_buffer().set(lhs, rhs); }

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditor: Protected functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void TextEditor::change_buffer(int32_t delta)
{   // {{{

    // Get the current buffer size.
    int32_t size = static_cast<int32_t>(this->hists_ref.size());

    // If the current buffer is the editing buffer, save it to the save buffer.
    if (this->index == size)
        this->saved_edit = this->buffer.serialize();

    // Update the buffer index.
    this->index = clip(this->index + delta, 0, size);

    // Update the current buffer data.
    if (this->index < size) { this->buffer.set(this->hists_ref[this->index], ""); }
    else                    { this->buffer.set(this->saved_edit,             ""); }

}   // }}}

GapBuffer& TextEditor::current_buffer(void)
{ return this->buffer; }

const GapBuffer& TextEditor::current_buffer(void) const
{ return this->buffer; }

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
