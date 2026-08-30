////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: text_editor.hxx                                                             ///
///                                                                                              ///
/// This file defines the class `TextBuffer` that manages user command editing.                  ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TEXT_EDITOR_HXX
#define TEXT_EDITOR_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"
#include "gap_buffer.hxx"
#include "utf8.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////////////////////////

inline constexpr char KEY_UP   [3] = {0x1b, 0x5b, 0x41};  // ^[[A => [0x1b,0x5b,0x41]
inline constexpr char KEY_DOWN [3] = {0x1b, 0x5b, 0x42};  // ^[[B => [0x1b,0x5b,0x42]
inline constexpr char KEY_RIGHT[3] = {0x1b, 0x5b, 0x43};  // ^[[C => [0x1b,0x5b,0x43]
inline constexpr char KEY_LEFT [3] = {0x1b, 0x5b, 0x44};  // ^[[D => [0x1b,0x5b,0x44]

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////////////////////////////////

class TextEditor
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Data types
        ////////////////////////////////////////////////////////////////////////////////////////////

        enum class Mode
        // Constants to represent state of buffer.
        // In the case of Emacs-style editor, there are two modes: insert mode and normal mode.
        {
            INSERT,  // Insert mode: new characters are inserted at the cursor position.
            NORMAL,  // Normal mode: keystrokes are interpreted as commands to manipulate the text buffer.
        };

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Character classification types (used by derived editor classes)
        ////////////////////////////////////////////////////////////////////////////////////////////

        enum class CharClass
        // Character classes used for word-motion calculations.
        {
            SPACE, // Space and tab characters.
            WORD,  // Alphanumeric characters, underscore, and all non-ASCII (multi-byte) characters.
            OTHER  // All other ASCII characters (punctuation, symbols, etc.)
        };

        struct CharInfo
        // Byte position and character class of one UTF-8 character inside a StringView.
        {
            SizeType  byte_pos;  // Byte offset of the character in the string (0-based).
            CharClass cls;       // Character class, used for word-motion boundary detection.
        };

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        TextEditor(StringView lhs, StringView rhs, const Deque<String>& hists);
        // Constructor with initial contents.
        //
        // [Args]
        //   lhs   (StringView)          : [IN] Left hand side text of the current editing buffer.
        //   rhs   (StringView)          : [IN] Right hand side text of the current editing buffer.
        //   hists (const Deque<String>&): [IN] Histories of text buffers.

        virtual ~TextEditor(void) = default;
        // Virtual destructor.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Getter and setter functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        StringView get_lhs(void) const;
        StringView get_rhs(void) const;
        // Get left/right hand side of the text buffer.

        TextEditor::Mode get_mode(void) const noexcept;
        // Get the current editing mode.

        void set(StringView lhs, StringView rhs);
        // Set left/right hand side of the current text buffer.
        //
        // [Args]
        //   lhs (StringView): [IN] Left hand side text to be set.
        //   rhs (StringView): [IN] Right hand side text to be set.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Edit functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        virtual void edit(StringView sv) = 0;
        // Edit buffer in the current mode.
        // This function is a pure virtual function and should be implemented in the derived class.
        //
        // [Args]
        //   sv (StringView): [IN] Input character as a string view.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Static utility functions (used by derived editor classes)
        ////////////////////////////////////////////////////////////////////////////////////////////

        static Vector<CharInfo> collect_chars(StringView sv);
        // Build a vector of (byte_pos, CharClass) for every UTF-8 character in sv.
        //
        // [Args]
        //   sv (StringView): [IN] Input string view to analyze.
        //
        // [Returns]
        //   (Vector<CharInfo>): CharInfo entry for each UTF-8 character in sv.

        static String extract_front(StringView sv, PtrDiff n);
        // Return the first n UTF-8 characters of sv as a new String.
        //
        // [Args]
        //   sv (StringView): [IN] Source string.
        //   n  (PtrDiff)   : [IN] Number of UTF-8 characters to extract from the front.
        //
        // [Returns]
        //   (String): First n UTF-8 characters, or all of sv if n >= character count.

        static String extract_back(StringView sv, PtrDiff n);
        // Return the last n UTF-8 characters of sv as a new String.
        //
        // [Args]
        //   sv (StringView): [IN] Source string.
        //   n  (PtrDiff)   : [IN] Number of UTF-8 characters to extract from the back.
        //
        // [Returns]
        //   (String): Last n UTF-8 characters, or all of sv if n >= character count.

    protected:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Protected member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        Vector<GapBuffer> buffers;
        // All text buffers.

        uint64_t index;
        // Current index of buffers.

        Mode mode;
        // Current editing mode.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Protected functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void change_buffer(int64_t delta);
        // Change the current buffer by the given delta.
        //
        // [Args]
        //   delta (int64_t): [IN] Delta to change the buffer index.

              GapBuffer& current_buffer(void);
        const GapBuffer& current_buffer(void) const;
        // Get the current editing buffer (and its const version).
        //
        // [Returns]
        //   (GapBuffer&): Current editing buffer.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
