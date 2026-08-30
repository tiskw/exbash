////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: text_editor.cxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "text_editor.hxx"

// Include the standard library headers.
#include <cctype>

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

TextEditor::TextEditor(StringView lhs, StringView rhs, const Deque<String>& hists) : mode(Mode::INSERT)
{   // {{{

    // Reserve space for the gap buffers.
    this->buffers.reserve(hists.size() + 1);

    // Register the buffer histories.
    for (const String& hist : hists)
        this->buffers.emplace_back(GapBuffer(hist, ""));

    // Create the current editing buffer.
    this->buffers.emplace_back(GapBuffer(lhs, rhs));
    this->index = this->buffers.size() - 1;

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditor: Protected static utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<TextEditor::CharInfo> TextEditor::collect_chars(StringView sv)
{   // {{{

    Vector<CharInfo> chars;
    SizeType p = 0;
    while (p < sv.size())
    {
        chars.push_back({p, classify_char(sv.data() + p)});
        uint8_t bsz = utf8_byte_size(static_cast<uint8_t>(sv[p]));
        p += (bsz > 0 && p + bsz <= sv.size()) ? bsz : 1;
    }
    return chars;

}   // }}}

String TextEditor::extract_front(StringView sv, PtrDiff n)
{   // {{{

    if (n <= 0 || sv.empty()) return "";
    SizeType pos   = 0;
    PtrDiff  count = 0;
    while (pos < sv.size() && count < n)
    {
        uint8_t bsz = utf8_byte_size(static_cast<uint8_t>(sv[pos]));
        pos += (bsz > 0 && pos + bsz <= sv.size()) ? bsz : 1;
        ++count;
    }
    return String(sv.data(), pos);

}   // }}}

String TextEditor::extract_back(StringView sv, PtrDiff n)
{   // {{{

    if (n <= 0 || sv.empty()) return "";
    const auto chars = collect_chars(sv);
    if (n >= static_cast<PtrDiff>(chars.size())) return String(sv);
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

void TextEditor::change_buffer(int64_t delta)
{   // {{{

    // Do nothing if there is no buffer.
    if (this->buffers.empty()) return;

    // Get the current buffer size.
    uint64_t size = this->buffers.size();

    // Update the buffer index.
    if (delta > 0)
        this->index = ((this->index + delta) < size) ? (this->index + delta) : (size - 1);
    if (delta < 0)
        this->index = (this->index >= static_cast<uint64_t>(-delta)) ? (this->index + delta) : 0;

}   // }}}

GapBuffer& TextEditor::current_buffer(void)
{ return this->buffers[this->index]; }

const GapBuffer& TextEditor::current_buffer(void) const
{ return this->buffers[this->index]; }

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
