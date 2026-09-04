////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: text_editor_vi.cxx                                                          ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "text_editor_vi.hxx"

// Include the headers of custom modules.
#include "utf8.hxx"

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

    Vector<CharInfo> collect_chars(StringView sv)  { return TextEditor::collect_chars(sv); }

    PtrDiff word_fwd_count(StringView rhs, bool bigword)
    // Return the number of UTF-8 characters to advance for w/W.
    {   // {{{

        if (rhs.empty()) return 0;
        const auto chars = collect_chars(rhs);
        if (chars.empty()) return 0;

        SizeType  idx   = 0;
        PtrDiff count = 0;

        if (bigword)
        {
            while (idx < chars.size() && chars[idx].cls != CharClass::SPACE) { ++idx; ++count; }
            while (idx < chars.size() && chars[idx].cls == CharClass::SPACE) { ++idx; ++count; }
        }
        else
        {
            CharClass start = chars[0].cls;
            if (start == CharClass::SPACE)
            {
                while (idx < chars.size() && chars[idx].cls == CharClass::SPACE) { ++idx; ++count; }
            }
            else
            {
                while (idx < chars.size() && chars[idx].cls == start)           { ++idx; ++count; }
                while (idx < chars.size() && chars[idx].cls == CharClass::SPACE) { ++idx; ++count; }
            }
        }
        return count;

    }   // }}}

    // Return the number of UTF-8 characters to move backward for b/B.
    PtrDiff word_bwd_count(StringView lhs, bool bigword)
    {   // {{{

        if (lhs.empty()) return 0;
        const auto chars = collect_chars(lhs);
        if (chars.empty()) return 0;

        int     idx   = static_cast<int>(chars.size()) - 1;
        PtrDiff count = 0;

        if (bigword)
        {
            while (idx >= 0 && chars[idx].cls == CharClass::SPACE) { --idx; ++count; }
            while (idx >= 0 && chars[idx].cls != CharClass::SPACE) { --idx; ++count; }
        }
        else
        {
            while (idx >= 0 && chars[idx].cls == CharClass::SPACE) { --idx; ++count; }
            if (idx < 0) return count;
            CharClass target = chars[idx].cls;
            while (idx >= 0 && chars[idx].cls == target)           { --idx; ++count; }
        }
        return count;

    }   // }}}

    PtrDiff word_end_count(StringView rhs, bool bigword)
    // Return the number of UTF-8 characters to advance to reach the end of the next word for e/E.
    // Returns 0 if the cursor is already at the last character of the last word.
    //
    {   // {{{

        if (rhs.empty()) return 0;
        const auto chars = collect_chars(rhs);
        if (chars.size() <= 1) return 0;  // already at end, or only one char

        SizeType  idx   = 1;
        PtrDiff count = 1;

        // Skip any leading spaces after current position.
        while (idx < chars.size() && chars[idx].cls == CharClass::SPACE) { ++idx; ++count; }
        if (idx >= chars.size()) return 0;

        // Advance to the last character of this word group.
        CharClass target = chars[idx].cls;
        while (idx + 1 < chars.size())
        {
            CharClass next = chars[idx + 1].cls;
            bool same_group = bigword ? (next != CharClass::SPACE) : (next == target);
            if (!same_group) break;
            ++idx; ++count;
        }
        return count;

    }   // }}}

    PtrDiff first_nonblank_pos(StringView lhs, StringView rhs)
    // Return the UTF-8 character position (count from line start) of the first non-blank character.
    //
    {   // {{{

        PtrDiff pos = 0;
        for (StringView sv : {lhs, rhs})
            for (const CharInfo& ci : collect_chars(sv))
            {
                if (ci.cls != CharClass::SPACE) return pos;
                ++pos;
            }
        return pos;

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditorVi: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

TextEditorVi::TextEditorVi(StringView lhs, StringView rhs, const Deque<String>& hists) : TextEditor(lhs, rhs, hists), pending_op(0)
{ /* Do nothing, initializer lists only. */ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditorVi: Edit functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void TextEditorVi::edit(StringView sv)
{   // {{{

    switch (this->mode)
    {
        case Mode::INSERT: this->edit_insert(sv.data(), sv.size()); break;
        case Mode::NORMAL: this->edit_normal(sv.data(), sv.size()); break;
    }

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TextEditorVi: Private functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void TextEditorVi::edit_insert(const char* str, SizeType size)
{   // {{{

    constexpr auto ins_ctrl = [](GapBuffer& buffer, char c) -> void
    // Insert the control character to the current buffer.
    //
    // [Args]
    //   buffer (GapBuffer&): [IN] Reference to the text editor instance.
    //   c      (char)      : [IN] Control character to be inserted.
    {
        // Convert the given character to the corresponding control character.
        const char str[3] = {'^', static_cast<char>(0x40 + c), '\0'};

        // Insert the control character to the current buffer.
        buffer.insert(str, 2);
    };

    // Get a reference to the current editing buffer, for convenience.
    GapBuffer& buffer = this->current_buffer();

    if (size == 1)
    {
        if (*str == 0x08) { buffer.backspace(1);    return; } // ^H (Backspace)
        if (*str == 0x7F) { buffer.backspace(1);    return; } // ^? (Backspace)
        if (*str == 0x1B) { mode = Mode::NORMAL;    return; } // ESC
        if (*str <= 0x1F) { ins_ctrl(buffer, *str); return; } // Control characters
    }

    if (size == 3)
    {
        if (std::memcmp(str, KEY_RIGHT, 3) == 0) { buffer.move_cursor(+1);  return; }
        if (std::memcmp(str, KEY_LEFT,  3) == 0) { buffer.move_cursor(-1);  return; }
        if (std::memcmp(str, KEY_DOWN,  3) == 0) { this->change_buffer(+1); return; }
        if (std::memcmp(str, KEY_UP,    3) == 0) { this->change_buffer(-1); return; }
    }

    buffer.insert(str, size);

}   // }}}

void TextEditorVi::edit_normal(const char* str, SizeType size)
{   // {{{

    // Get a reference to the current editing buffer, for convenience.
    GapBuffer& buffer = this->current_buffer();

    // Handle arrow-key escape sequences.
    if (size == 3)
    {
        if (std::memcmp(str, KEY_RIGHT, 3) == 0) { buffer.move_cursor(+1);  return; }
        if (std::memcmp(str, KEY_LEFT,  3) == 0) { buffer.move_cursor(-1);  return; }
        if (std::memcmp(str, KEY_DOWN,  3) == 0) { this->change_buffer(+1); return; }
        if (std::memcmp(str, KEY_UP,    3) == 0) { this->change_buffer(-1); return; }
        return;
    }

    if (size != 1) return;

    const char ch = *str;

    // ESC cancels any pending operator without performing any action.
    if (ch == 0x1B) { this->pending_op = 0; return; }

    // If an operator is pending (d/c/y/r), delegate to handle_pending.
    if (this->pending_op != 0) { this->handle_pending(ch); return; }

    // ----------------------------------------------------
    // Cursor movement
    // ----------------------------------------------------
    if (ch == 'h') { buffer.move_cursor(-1);  return; }
    if (ch == 'l') { buffer.move_cursor(+1);  return; }
    if (ch == '0') { buffer.move_top();       return; }
    if (ch == '$') { buffer.move_end();       return; }
    if (ch == '^')
    {
        PtrDiff target = first_nonblank_pos(buffer.lhs_view(), buffer.rhs_view());
        buffer.move_cursor(target - static_cast<PtrDiff>(buffer.cursor()));
        return;
    }

    // ----------------------------------------------------
    // Word motions
    // ----------------------------------------------------
    if (ch == 'w') { buffer.move_cursor( word_fwd_count(buffer.rhs_view(), false)); return; }
    if (ch == 'W') { buffer.move_cursor( word_fwd_count(buffer.rhs_view(), true )); return; }
    if (ch == 'b') { buffer.move_cursor(-word_bwd_count(buffer.lhs_view(), false)); return; }
    if (ch == 'B') { buffer.move_cursor(-word_bwd_count(buffer.lhs_view(), true )); return; }
    if (ch == 'e') { buffer.move_cursor( word_end_count(buffer.rhs_view(), false)); return; }
    if (ch == 'E') { buffer.move_cursor( word_end_count(buffer.rhs_view(), true )); return; }

    // ----------------------------------------------------
    // History navigation
    // ----------------------------------------------------
    if (ch == 'j') { this->change_buffer(+1); return; }
    if (ch == 'k') { this->change_buffer(-1); return; }

    // ----------------------------------------------------
    // Mode transitions
    // ----------------------------------------------------
    if (ch == 'i') {                         mode = Mode::INSERT; return; }
    if (ch == 'I') { buffer.move_top();      mode = Mode::INSERT; return; }
    if (ch == 'a') { buffer.move_cursor(+1); mode = Mode::INSERT; return; }
    if (ch == 'A') { buffer.move_end();      mode = Mode::INSERT; return; }
    if (ch == 's') { buffer.deletekey(1);    mode = Mode::INSERT; return; }
    if (ch == 'C') // Change to end of line (equivarent to 'c$').
    {
        this->yank_buffer = String(buffer.rhs_view());
        buffer.erase_rhs();
        mode = Mode::INSERT;
        return;
    }

    // ----------------------------------------------------
    // Single-character edits
    // ----------------------------------------------------
    if (ch == 'x') { buffer.deletekey(1); return; }
    if (ch == 'X') { buffer.backspace(1); return; }
    if (ch == 'S') // Substitute whole line.
    {
        this->yank_buffer = String(buffer.lhs_view()) + String(buffer.rhs_view());
        buffer.erase();
        mode = Mode::INSERT;
        return;
    }
    if (ch == 'D') // Delete to end.
    {
        this->yank_buffer = String(buffer.rhs_view());
        buffer.erase_rhs();
        return;
    }
    if (ch == '~') // Toggle case of char under cursor.
    {
        StringView rhs = buffer.rhs_view();
        if (!rhs.empty())
        {
            uint8_t c = static_cast<uint8_t>(rhs[0]);
            if (c < 0x80 && std::isalpha(static_cast<unsigned char>(c)))
            {
                char toggled = std::isupper(static_cast<unsigned char>(c))
                             ? static_cast<char>(std::tolower(static_cast<unsigned char>(c)))
                             : static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                buffer.deletekey(1);
                buffer.insert(&toggled, 1);
            }
            else
            {
                buffer.move_cursor(+1); // non-alpha: just advance
            }
        }
        return;
    }

    // ----------------------------------------------------
    // Paste
    // ----------------------------------------------------
    if (ch == 'p') // paste after cursor
    {
        if (not buffer.rhs_view().empty()) buffer.move_cursor(+1);
        if (not this->yank_buffer.empty()) buffer.insert(this->yank_buffer);
        return;
    }
    if (ch == 'P') // paste before cursor
    {
        if (not this->yank_buffer.empty()) buffer.insert(this->yank_buffer);
        return;
    }

    // ----------------------------------------------------
    // Operators (set pending)
    // ----------------------------------------------------
    if (ch == 'd' || ch == 'c' || ch == 'y' || ch == 'r') { this->pending_op = ch; return; }

}   // }}}

void TextEditorVi::handle_pending(char motion)
{   // {{{

    // Get the pending operator and clear it immediately to avoid re-entrancy issues.
    const char op = this->pending_op;
    this->pending_op = 0;

    // Get a reference to the current editing buffer, for convenience.
    GapBuffer& buffer = this->current_buffer();

    // ---- r: replace single character ----
    if (op == 'r')
    {
        if (!buffer.rhs_view().empty())
        {
            buffer.deletekey(1);
            buffer.insert(&motion, 1);
            buffer.move_cursor(-1); // stay on replaced character
        }
        return;
    }

    // ---- d/c/y: operator + motion ----

    // Whole-line: dd, cc, yy
    if (motion == op)
    {
        this->yank_buffer = String(buffer.lhs_view()) + String(buffer.rhs_view());
        if (op == 'd' || op == 'c') buffer.erase();
        if (op == 'c') this->mode = Mode::INSERT;
        return;
    }

    const StringView lhs = buffer.lhs_view();
    const StringView rhs = buffer.rhs_view();

    // Helper: apply operator on the next n chars of rhs.
    auto apply_fwd = [&](PtrDiff n)
    {
        if (n <= 0) return;
        this->yank_buffer = extract_front(rhs, n);
        if (op == 'd' || op == 'c') buffer.deletekey(n);
        if (op == 'c') this->mode = Mode::INSERT;
    };

    // Helper: apply operator on the last n chars of lhs.
    auto apply_bwd = [&](PtrDiff n)
    {
        if (n <= 0) return;
        this->yank_buffer = extract_back(lhs, n);
        if (op == 'd' || op == 'c') buffer.backspace(n);
        if (op == 'c') this->mode = Mode::INSERT;
    };

    switch (motion)
    {
        case 'w': apply_fwd(word_fwd_count(rhs, false)); break;
        case 'W': apply_fwd(word_fwd_count(rhs, true));  break;
        case 'b': apply_bwd(word_bwd_count(lhs, false)); break;
        case 'B': apply_bwd(word_bwd_count(lhs, true));  break;
        case 'e': { PtrDiff n = word_end_count(rhs, false); if (n > 0) apply_fwd(n + 1); break; }
        case 'E': { PtrDiff n = word_end_count(rhs, true);  if (n > 0) apply_fwd(n + 1); break; }
        case '0': apply_bwd(static_cast<PtrDiff>(buffer.cursor())); break;
        case '$': apply_fwd(static_cast<PtrDiff>(buffer.count() - buffer.cursor())); break;
        case '^':
        {
            PtrDiff target  = first_nonblank_pos(lhs, rhs);
            PtrDiff current = static_cast<PtrDiff>(buffer.cursor());
            PtrDiff delta   = target - current;
            if      (delta > 0) apply_fwd( delta);
            else if (delta < 0) apply_bwd(-delta);
            break;
        }
        // Unknown motion character: silently cancel (pending_op already cleared).
    }

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
