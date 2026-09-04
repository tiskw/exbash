////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: text_editor_emacs.hxx                                                       ///
///                                                                                              ///
/// Emacs-like text editor that inherits from TextEditor and implements Emacs-style keybindings. ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TEXT_EDITOR_EMACS_HXX
#define TEXT_EDITOR_EMACS_HXX

// Include the headers of custom modules.
#include "text_editor.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////////////////////////////////

class TextEditorEmacs : public TextEditor
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        TextEditorEmacs(StringView lhs, StringView rhs, const Deque<String>& hists);
        // Constructor with initial contents and histories.
        //
        // [Args]
        //   lhs   (StringView)          : [IN] Left hand side text of the current editing buffer.
        //   rhs   (StringView)          : [IN] Right hand side text of the current editing buffer.
        //   hists (const Deque<String>&): [IN] Histories of text buffers.

        ~TextEditorEmacs(void) override = default;
        // Default destructor.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Edit functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void edit(StringView sv) override;
        // Edit buffer. Emacs mode is always in "insert" state; there is no NORMAL mode.
        //
        // [Args]
        //   sv (StringView): [IN] Input character as a string view.

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        bool meta_pending;
        // True when ESC (Meta prefix) has been received and we are waiting for the next key.

        String kill_ring;
        // Single-slot kill ring; filled by C-k/C-u/C-w/M-d/M-DEL and pasted by C-y.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void process(const char* str, SizeType size);
        // Process one raw key event in the normal (non-meta) state.
        //
        // [Args]
        //   str  (const char*): [IN] Bytes of the received key.
        //   size (SizeType)   : [IN] Number of bytes.

        void process_meta(char ch);
        // Process one character that follows an ESC (Meta) prefix.
        //
        // [Args]
        //   ch (char): [IN] The character pressed after ESC.

        void transpose_chars(void);
        // Swap the character before the cursor with the character under the cursor (C-t).
        // When the cursor is at the end of the line, the last two characters are swapped.

        void word_case_transform(int wcase);
        // Transform the case of the word starting at the current cursor position.
        // Moves the cursor to the end of the transformed word.
        //
        // [Args]
        //   wcase (int): [IN] 0 = UPPER, 1 = lower, 2 = Capitalize.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
