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

    PtrDiff word_fwd_count(StringView rhs, bool bigword)
    // Return the number of UTF-8 characters to advance for w/W.
    //
    // [Args]
    //   rhs     (StringView): [IN] The right-hand-side string to analyze.
    //   bigword (bool)      : [IN] If true, treat all non-space characters as a single word (W),
    //                              otherwise treat only WORD characters as a word (w).
    //
    // [Returns]
    //   (PtrDiff): Number of UTF-8 characters to advance to reach the start of the next word.
    //
    {   // {{{

        // If the input string is empty, there are no characters to skip.
        if (rhs.empty()) return 0;

        // Get the character info vector for rhs.
        const Vector<CharInfo> chars = TextEditor::collect_char_info(rhs);
        if (chars.empty()) return 0;

        // Initialize the index and count variables.
        SizeType idx   = 0;
        PtrDiff  count = 0;

        if (bigword)
        {
            // Skip any leading non-space characters, then skip any space characters.
            while ((idx < chars.size()) and (chars[idx].cls != CharClass::SPACE)) { ++idx; ++count; }
            while ((idx < chars.size()) and (chars[idx].cls == CharClass::SPACE)) { ++idx; ++count; }
        }
        else
        {
            // Get the character class of the first character in rhs.
            CharClass start = chars[0].cls;

            // If the first character is a space, skip all leading spaces.
            if (start == CharClass::SPACE)
            {
                while (idx < chars.size() && chars[idx].cls == CharClass::SPACE) { ++idx; ++count; }
            }
            // Otherwise, skip all characters of the same class as the first character, then skip any trailing spaces.
            else
            {
                while (idx < chars.size() && chars[idx].cls == start)           { ++idx; ++count; }
                while (idx < chars.size() && chars[idx].cls == CharClass::SPACE) { ++idx; ++count; }
            }
        }

        return count;

    }   // }}}

    PtrDiff word_bwd_count(StringView lhs, bool bigword)
    // Return the number of UTF-8 characters to move backward for b/B.
    //
    // [Args]
    //   lhs     (StringView): [IN] The left-hand-side string to analyze.
    //   bigword (bool)      : [IN] If true, treat all non-space characters as a single word (B),
    //                              otherwise treat only WORD characters as a word (b).
    //
    // [Returns]
    //   (PtrDiff): Number of UTF-8 characters to move backward to reach the start of the previous word.
    //
    {   // {{{

        // If the input string is empty, there are no characters to skip.
        if (lhs.empty()) return 0;

        // Get the character info vector for lhs.
        const Vector<CharInfo> chars = TextEditor::collect_char_info(lhs);
        if (chars.empty()) return 0;

        // Initialize the index and count variables.
        int32_t idx   = static_cast<int>(chars.size()) - 1;
        PtrDiff count = 0;

        if (bigword)
        {
            // Skip any trailing spaces, then skip any non-space characters.
            while ((idx >= 0) and (chars[idx].cls == CharClass::SPACE)) { --idx; ++count; }
            while ((idx >= 0) and (chars[idx].cls != CharClass::SPACE)) { --idx; ++count; }
        }
        else
        {
            // Skip any trailing spaces.
            while ((idx >= 0) and (chars[idx].cls == CharClass::SPACE))
            { --idx; ++count; }

            // Return the count if we have reached the beginning of the string.
            if (idx < 0) return count;

            // Get the character class of the last non-space character in lhs, and skip all characters of that class.
            CharClass target = chars[idx].cls;
            while ((idx >= 0) and (chars[idx].cls == target))
            { --idx; ++count; }
        }

        return count;

    }   // }}}

    PtrDiff word_end_count(StringView rhs, bool bigword)
    // Return the number of UTF-8 characters to advance to reach the end of the next word for e/E.
    // Returns 0 if the cursor is already at the last character of the last word.
    //
    // [Args]
    //   rhs     (StringView): [IN] The right-hand-side string to analyze.
    //   bigword (bool)      : [IN] If true, treat all non-space characters as a single word (E),
    //                              otherwise treat only WORD characters as a word (e).
    //
    // [Returns]
    //   (PtrDiff): Number of UTF-8 characters to advance to reach the end of the next word.
    //
    {   // {{{

        // If the input string is empty, there are no characters to skip.
        if (rhs.empty()) return 0;

        // Get the character info vector for rhs.
        const Vector<CharInfo> chars = TextEditor::collect_char_info(rhs);

        // If the input string is empty or has only one character, there are no characters to skip.
        if (chars.size() <= 1) return 0;

        // Initialize the index and count variables.
        SizeType idx   = 1;
        PtrDiff  count = 1;

        // Skip any leading spaces after current position.
        while (idx < chars.size() && chars[idx].cls == CharClass::SPACE) { ++idx; ++count; }

        // Return 0 if we have reached the end of the string.
        if (idx >= chars.size()) return 0;

        // Advance to the last character of this word group.
        CharClass target = chars[idx].cls;
        while ((idx + 1) < chars.size())
        {
            // Get the character class of the next character.
            CharClass next = chars[idx + 1].cls;

            // Determine if the next character belongs to the same word group as the current character.
            bool is_same_group = bigword ? (next != CharClass::SPACE) : (next == target);

            // Break the loop if the next character does not belong to the same word group.
            if (not is_same_group) break;

            // Increment both index and count.
            ++idx; ++count;
        }
        return count;

    }   // }}}

    PtrDiff first_nonblank_pos(StringView lhs, StringView rhs)
    // Return the UTF-8 character position (count from line start) of the first non-blank character.
    //
    // [Args]
    //   lhs (StringView): [IN] The left-hand-side string to analyze.
    //   rhs (StringView): [IN] The right-hand-side string to analyze.
    //
    // [Returns]
    //   (PtrDiff): UTF-8 character position of the first non-blank character, or the total character count if all are blank.
    //
    {   // {{{

        // Initialize the position counter.
        PtrDiff pos = 0;

        // Iterate over both lhs and rhs.
        for (StringView sv : {lhs, rhs})
        {
            // Iterate over the characters in the current string view.
            for (const CharInfo& ci : TextEditor::collect_char_info(sv))
            {
                // Return the current position if the character class is not SPACE.
                if (ci.cls != CharClass::SPACE)
                    return pos;

                ++pos;
            }
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

        // Do nothing if the right-hand side is empty (no character under cursor).
        if (rhs.empty()) return;

        // Get the first character of rhs and check if it is an ASCII letter.
        uint8_t c = static_cast<uint8_t>(rhs[0]);

        // If it is an ASCII letter, toggle its case and replace it in the buffer.
        if ((c < 0x80) and std::isalpha(static_cast<unsigned char>(c)))
        {
            // Toggle the case of the character.
            char c_toggled = std::isupper(static_cast<unsigned char>(c))
                           ? static_cast<char>(std::tolower(static_cast<unsigned char>(c)))
                           : static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

            // Replace the character under the cursor with the toggled character.
            buffer.deletekey(1);
            buffer.insert(&c_toggled, 1);
        }

        // Otherwise, simply move the cursor forward.
        else { buffer.move_cursor(+1); }

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

    constexpr auto apply_fwd = [](TextEditorVi* self, const char op, PtrDiff n) -> void
    // Apply operator on the next n chars of rhs.
    // This function is used for operators with forward motions.
    //
    // [Args]
    //   self (TextEditorVi*): [IN] Pointer to the TextEditorVi instance.
    //   op   (char)         : [IN] Operator character ('d', 'c', or 'y').
    //   n    (PtrDiff)      : [IN] Number of characters to apply the operator on.
    {
        // Get a reference to the current editing buffer and the right-hand-side view.
        GapBuffer&       buffer = self->current_buffer();
        const StringView rhs    = buffer.rhs_view();

        // Do nothing if n is invalid (= non-positive).
        if (n <= 0) return;

        // Yank the next n characters of rhs into the yank buffer.
        self->yank_buffer = extract_front(rhs, n);

        // Apply the operator to the next n characters of rhs.
        if (op == 'd' || op == 'c') buffer.deletekey(n);

        // Mode transition.
        if (op == 'c') self->mode = Mode::INSERT;
    };

    constexpr auto apply_bwd = [](TextEditorVi* self, const char op, PtrDiff n) -> void
    // Apply operator on the last n chars of lhs.
    // This function is used for operators with backward motions.
    //
    // [Args]
    //   self (TextEditorVi*): [IN] Pointer to the TextEditorVi instance.
    //   op   (char)         : [IN] Operator character ('d', 'c', or 'y').
    //   n    (PtrDiff)      : [IN] Number of characters to apply the operator on.
    {
        // Get a reference to the current editing buffer and the right-hand-side view.
        GapBuffer&       buffer = self->current_buffer();
        const StringView lhs    = buffer.lhs_view();

        // Do nothing if n is invalid (= non-positive).
        if (n <= 0) return;

        // Yank the last n characters of lhs into the yank buffer.
        self->yank_buffer = extract_back(lhs, n);

        // Apply the operator to the last n characters of lhs.
        if (op == 'd' || op == 'c') buffer.backspace(n);

        // Mode transition.
        if (op == 'c') self->mode = Mode::INSERT;
    };

    // Get the pending operator and clear it immediately to avoid re-entrancy issues.
    const char op = this->pending_op;
    this->pending_op = 0;

    // Get a reference to the current editing buffer, for convenience.
    GapBuffer& buffer = this->current_buffer();

    // ----------------------------------------------------
    // Operation only (r)
    // ----------------------------------------------------

    if (op == 'r')
    {
        if (not buffer.rhs_view().empty())
        {
            buffer.deletekey(1);
            buffer.insert(&motion, 1);
            buffer.move_cursor(-1);
        }
        return;
    }

    // ----------------------------------------------------
    // Operation and motion (c/d/y)
    // ----------------------------------------------------

    if ((op == 'c') or (op == 'd') or (op == 'y'))
    {
        // Whole-line operations (dd/cc/yy).
        if (motion == op)
        {
            // Yank the entire line (lhs + rhs).
            this->yank_buffer = String(buffer.lhs_view()) + String(buffer.rhs_view());

            // Apply the operation to the entire line.
            if (op == 'd' || op == 'c') buffer.erase();

            // Mode transition.
            if (op == 'c') this->mode = Mode::INSERT;

            return;
        }

        // Get the left-hand-side and right-hand-side views of the buffer.
        const StringView lhs = buffer.lhs_view();
        const StringView rhs = buffer.rhs_view();

        // Get the total character count and cursor position of the buffer.
        const PtrDiff buf_count  = static_cast<PtrDiff>(buffer.count());
        const PtrDiff buf_cursor = static_cast<PtrDiff>(buffer.cursor());

        switch (motion)
        {
            case 'w': apply_fwd(this, op, word_fwd_count(rhs, false)    ); break;
            case 'W': apply_fwd(this, op, word_fwd_count(rhs, true )    ); break;
            case 'b': apply_bwd(this, op, word_bwd_count(lhs, false)    ); break;
            case 'B': apply_bwd(this, op, word_bwd_count(lhs, true )    ); break;
            case 'e': apply_fwd(this, op, word_end_count(rhs, false) + 1); break;
            case 'E': apply_fwd(this, op, word_end_count(rhs, true ) + 1); break;
            case '0': apply_bwd(this, op, buf_cursor                    ); break;
            case '$': apply_fwd(this, op, buf_count - buf_cursor        ); break;
            case '^':
            {
                PtrDiff target  = first_nonblank_pos(lhs, rhs);
                PtrDiff current = static_cast<PtrDiff>(buffer.cursor());
                PtrDiff delta   = target - current;
                if      (delta > 0) apply_fwd(this, op, +delta);
                else if (delta < 0) apply_bwd(this, op, -delta);
                break;
            }

            // Unknown motion characters are ignored silently (pending_op already cleared).
            default: break;
        }
    }

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
