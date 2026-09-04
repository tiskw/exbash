////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: gap_buffer.hxx                                                              ///
///                                                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GAP_BUFFER_HXX
#define GAP_BUFFER_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////////////////////////////////

class GapBuffer
{
    // C++ implementation of a gap buffer for text editing.
    // A gap buffer is a buffer data structure that allows efficient insertion and deletion of texts.
    //
    // The gap buffer manages the buffer of "before cursor" and "after cursor" with a gap.
    // The position of the gap is managed by two indices: gap_idx_top and gap_idx_end as below.
    //
    // [0,           gap_idx_top) ... a buffer before the cursor.
    // [gap_idx_top, gap_idx_end) ... a gap (contents of this area is meaningless).
    // [gap_idx_end, end        ) ... a buffer after the cursor.

    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        GapBuffer(StringView lhs, StringView rhs);
        // Constructor with initial contents.
        //
        // [Args]
        //   lhs (StringView): [IN] Initial left-hand-side text.
        //   rhs (StringView): [IN] Initial right-hand-side text.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Getter and setter functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        SizeType capacity(void) const noexcept;
        // Get the capacity of the buffer (including the gap).
        //
        // [Returns]
        //   (SizeType): Capacity of the buffer.

        SizeType count(void) const noexcept;
        // Get the number of UTF-8 characters in the buffer.
        //
        // [Returns]
        //   (SizeType): Number of UTF-8 characters in the buffer.

        SizeType cursor(void) const noexcept;
        // Get the cursor position.
        //
        // [Returns]
        //   (SizeType): Cursor position.

        StringView lhs_view(void) const noexcept;
        StringView rhs_view(void) const noexcept;
        // Get a view of the left/right-hand-side buffer and its size.
        //
        // [Notes]
        //   The return value of this function is a string view, not a copy of the string data,
        //   and will become invalid if the string data instance is reallocated.

        String serialize(void) const;
        // Dump the buffer contents as a serialized vector.
        //
        // [Returns]
        //   (String): The serialized buffer contents.

        void set(StringView lhs, StringView rhs);
        // Reset the buffer with new left/right-hand-side contents.
        //
        // [Args]
        //   lhs (StringView): [IN] New left-hand-side text.
        //   rhs (StringView): [IN] New right-hand-side text.

        SizeType size(void) const noexcept;
        // Get the size of the buffer contents (excluding the gap).
        //
        // [Returns]
        //   (SizeType): Size of the buffer contents.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Text editing functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void backspace(PtrDiff n);
        // Delete n characters before the cursor position.
        //
        // [Args]
        //   n (SizeType): [IN] Number of characters to be deleted.

        void deletekey(PtrDiff n);
        // Delete n characters after the cursor position.
        //
        // [Args]
        //   n (SizeType): [IN] Number of characters to be deleted.

        void erase(void);
        // Erase all characters in the buffer.

        void erase_lhs(void);
        // Erase all characters before the cursor position.

        void erase_rhs(void);
        // Erase all characters after the cursor position.

        void insert(const char* str, SizeType size = 0);
        // Insert a character at the cursor position and move the cursor forward.
        //
        // [Args]
        //   str  (const CharX&): [IN] String to be inserted.
        //   size (SizeType)    : [IN] Size of the input string.

        void insert(StringView str);
        // Insert a string at the cursor position and move the cursor forward.
        //
        // [Args]
        //   str (StringX&): [IN] String to be inserted.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Cursor movement functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void move_cursor(PtrDiff delta);
        // Move the cursor by the given delta.
        //
        // [Args]
        //   delta (ptrdiff_t): [IN] Delta to move the cursor.
        //
        // [Notes]
        //   Positive value moves the cursor forward, and negative value moves the cursor backward.

        void move_top(void);
        void move_end(void);
        // Move the cursor to the top/end of the buffer.

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        String buffer;
        // The buffer storage.

        PtrDiff gap_idx_top;
        PtrDiff gap_idx_end;
        // Index to point the border of the buffer contents and the gap.

        Vector<PtrDiff> utf8idxs;
        // Pointer to the first byte of each UTF-8 character.

        Vector<uint8_t> utf8wids;
        // Width of the UTF-8 characters.

        PtrDiff count_left;
        // Number of UTF-8 characters in the left side buffer.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void append_utf8_indices(StringView sv, PtrDiff offset);
        // Append the UTF-8 indices and widths of the given string to the end of the UTF-8 indices and widths vectors.
        //
        // [Args]
        //   sv     (StringView): [IN] The string to be appended.
        //   offset (PtrDiff)   : [IN] The offset to be added to the UTF-8 indices.

        void ensure_gap(SizeType need);
        // Ensure that the gap has enough size.
        //
        // [Args]
        //   need (SizeType): [IN] Required size of the gap

        SizeType gap_size(void) const noexcept;
        // Get the size of the gap.
        //
        // [Returns]
        //   (SizeType): Size of the gap.

        SizeType size_right(void) const noexcept;
        // Get the size of the buffer after the cursor.
        //
        // [Returns]
        //   (SizeType): Size of the buffer after the cursor.

        PtrDiff count_right(void) const noexcept;
        // Get the number of UTF-8 characters in the right side buffer.
        //
        // [Returns]
        //   (PtrDiff): Number of UTF-8 characters in the right side buffer.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
