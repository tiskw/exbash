////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: text_editor_vi.hxx                                                          ///
///                                                                                              ///
/// Vi-like text editor that inherits from TextEditor and implements Vi-style editing commands.  ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TEXT_EDITOR_VI_HXX
#define TEXT_EDITOR_VI_HXX

// Include the headers of custom modules.
#include "text_editor.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////////////////////////////////

class TextEditorVi : public TextEditor
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        TextEditorVi(StringView lhs, StringView rhs, const Deque<String>& hists);
        // Constructor with initial contents and histories.
        //
        // [Args]
        //   lhs   (StringView)          : [IN] Left hand side text of the current editing buffer.
        //   rhs   (StringView)          : [IN] Right hand side text of the current editing buffer.
        //   hists (const Deque<String>&): [IN] Histories of text buffers.

        ~TextEditorVi(void) override = default;
        // Default destructor.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Edit functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void edit(StringView sv) override;
        // Edit buffer in the current mode.
        //
        // [Args]
        //   sv (StringView): [IN] Input character as a string view.

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        char pending_op;
        // Pending operator character: 0 (none), 'd', 'c', 'y', or 'r'.

        String yank_buffer;
        // Yank buffer for copy/paste operations (p/P commands).

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void edit_insert(const char* str, SizeType size = 0);
        // Edit buffer in INSERT mode.
        //
        // [Args]
        //   str  (const char*): [IN] Input character as a C-style string.
        //   size (SizeType)   : [IN] Size of the input string.

        void edit_normal(const char* str, SizeType size = 0);
        // Edit buffer in NORMAL mode.
        //
        // [Args]
        //   str  (const char*): [IN] Input character as a C-style string.
        //   size (SizeType)   : [IN] Size of the input string.

        void handle_pending(char motion);
        // Apply a pending operator (d/c/y/r) with the given motion character.
        //
        // [Args]
        //   motion (char): [IN] Motion character pressed after the operator.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
