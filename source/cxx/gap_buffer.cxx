////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: gap_buffer.cxx                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "gap_buffer.hxx"

// Include STL headers.
#include <algorithm>
#include <utility>

// Include the headers of custom modules.
#include "utf8.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// GapBuffer: Constructors
////////////////////////////////////////////////////////////////////////////////////////////////////

GapBuffer::GapBuffer(StringView lhs, StringView rhs)
{ this->set(lhs, rhs); }

////////////////////////////////////////////////////////////////////////////////////////////////////
// GapBuffer: Getter and setter functions
////////////////////////////////////////////////////////////////////////////////////////////////////

SizeType GapBuffer::capacity(void) const noexcept
{ return this->buffer.capacity(); }

SizeType GapBuffer::count(void) const noexcept
{ return this->utf8idxs.size(); }

SizeType GapBuffer::cursor(void) const noexcept
{ return this->count_left; }

StringView GapBuffer::lhs_view(void) const noexcept
{ return StringView(this->buffer.data(), this->gap_idx_top); }

StringView GapBuffer::rhs_view(void) const noexcept
{ return StringView(this->buffer.data() + this->gap_idx_end, this->buffer.size() - this->gap_idx_end); }

String GapBuffer::serialize(void) const
{   // {{{

    // Create a new String instance to store the serialized buffer,
    // and reserve enough size to avoid multiple reallocations.
    String str;
    str.reserve(this->size() + 32);

    // Copy left and right parts to the new buffer.
    str.insert(str.end(), this->buffer.begin(), this->buffer.begin() + this->gap_idx_top);
    str.insert(str.end(), this->buffer.begin() + this->gap_idx_end, this->buffer.end());

    return str;

}   // }}}

void GapBuffer::set(StringView lhs, StringView rhs)
{   // {{{

    // Make copies of the given string views to ensure that the string data will not be invalidated during the buffer reallocation.
    const String lhs_copy(lhs), rhs_copy(rhs);

    // Resize the buffer if the current size is not enough.
    if (this->buffer.size() < (lhs.size() + rhs.size() + 32))
        this->buffer.resize(max((lhs.size() + rhs.size()) * 2, static_cast<SizeType>(32)));

    // Initialize gap indices.
    this->gap_idx_top = lhs.size();
    this->gap_idx_end = this->buffer.size() - rhs.size();

    // Place the left buffer on [0, gap_beg).
    std::copy(lhs_copy.begin(), lhs_copy.end(), this->buffer.begin());

    // Place the right buffer on [gap_end, end).
    std::copy(rhs_copy.begin(), rhs_copy.end(), this->buffer.begin() + this->gap_idx_end);

    // Clear the UTF-8 indices and widths to construct the UTF-8 indices from scratch.
    this->utf8idxs.clear();
    this->utf8wids.clear();

    // Construct the UTF-8 indices of the left side.
    this->append_utf8_indices(lhs_copy, 0);

    // It's good timing to get the number of UTF-8 characters in the left side.
    this->count_left = this->utf8idxs.size();

    // Construct the UTF-8 indices of the right side.
    this->append_utf8_indices(rhs_copy, this->gap_idx_end);

}   // }}}

SizeType GapBuffer::size(void) const noexcept
{ return this->buffer.size() - this->gap_size(); }

////////////////////////////////////////////////////////////////////////////////////////////////////
// GapBuffer: Text editing functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void GapBuffer::backspace(PtrDiff n)
{   // {{{

    // Do nothing if the given number of backspace is negative, or no character exists at the left of the cursor.
    if ((n < 0) or (this->count_left == 0)) return;

    // Calculate the index of the UTF-8 character to backspace.
    SizeType idx = (this->count_left > n) ? (this->count_left - n) : 0;

    // Update the top index of the gap.
    this->gap_idx_top = (idx < this->utf8idxs.size()) ? this->utf8idxs[idx] : 0;

    // Remove the UTF-8 indices and widths of the backspaced characters.
    this->utf8idxs.erase(this->utf8idxs.begin() + idx, this->utf8idxs.begin() + this->count_left);
    this->utf8wids.erase(this->utf8wids.begin() + idx, this->utf8wids.begin() + this->count_left);

    // Update the number of UTF-8 characters in the left side.
    this->count_left = idx;

}   // }}}

void GapBuffer::deletekey(PtrDiff n)
{   // {{{

    // Do nothing if the given number of delete is negative, or no character exists at the right of the cursor.
    if ((n < 0) or (this->count_right() == 0)) return;

    // Calculate the index of the UTF-8 character to delete.
    SizeType idx = min(this->count_left + static_cast<SizeType>(n), this->utf8idxs.size());

    // Update the end index of the gap.
    this->gap_idx_end = (idx < this->utf8idxs.size()) ? this->utf8idxs[idx] : this->buffer.size();

    // Remove the UTF-8 indices and widths of the deleted characters.
    this->utf8idxs.erase(this->utf8idxs.begin() + this->count_left, this->utf8idxs.begin() + idx);
    this->utf8wids.erase(this->utf8wids.begin() + this->count_left, this->utf8wids.begin() + idx);

}   // }}}

void GapBuffer::erase(void)
{   // {{{

    // To erase the entire gap buffer.
    this->gap_idx_top = 0;
    this->gap_idx_end = this->buffer.size();

    // Remove the UTF-8 indices and widths of the left side.
    this->utf8idxs.clear();
    this->utf8wids.clear();

    // Update the number of UTF-8 characters in the left side.
    this->count_left = 0;

}   // }}}

void GapBuffer::erase_lhs(void)
{   // {{{

    // To erase the left part of the gap buffer, simply move the top index of the gap to the beginning of the buffer.
    this->gap_idx_top = 0;

    // Remove the UTF-8 indices and widths of the left side.
    this->utf8idxs.erase(this->utf8idxs.begin(), this->utf8idxs.begin() + this->count_left);
    this->utf8wids.erase(this->utf8wids.begin(), this->utf8wids.begin() + this->count_left);

    // Update the number of UTF-8 characters in the left side.
    this->count_left = 0;

}   // }}}

void GapBuffer::erase_rhs(void)
{   // {{{

    // To erase the right part of the gap buffer, simply move the end index of the gap to the end of the buffer.
    this->gap_idx_end = this->buffer.size();

    // Remove the UTF-8 indices and widths of the right side.
    this->utf8idxs.erase(this->utf8idxs.begin() + this->count_left, this->utf8idxs.end());
    this->utf8wids.erase(this->utf8wids.begin() + this->count_left, this->utf8wids.end());

}   // }}}

void GapBuffer::insert(const char* str, SizeType size)
{   // {{{

    // Notes: Repetition of "this->utf8idxs.insert" and "this->utf8wids.insert" is high cost operation, and the common
    // approach to avoid it is to create a temporary vector instances to store the UTF-8 indices and widths of
    // the inserted string, and then insert them at once. However, this gap buffer class will be used as a buffer of
    // text editor or similar application, and the inserted string is expected to be a single character. In this case,
    // the current implementation is efficient enough compared to the common approach because the creation of temporary
    // vector instances is more costly than the "insert".

    // Do nothing if the given string is nullptr.
    if (str == nullptr) return;

    // Recalculate the size if the size is zero (= default value).
    if (size == 0)
        size = std::strlen(str);

    // Do nothing if the size is still zero.
    if (size == 0) return;

    // Make sure the gap has enough size.
    this->ensure_gap(size);

    // Copy the given string to the gap.
    std::memcpy(this->buffer.data() + this->gap_idx_top, str, size);

    for (const auto& [codepoint, ptr] : utf8_decode_iter(str, size))
    {
        // Calculate the index and width of the UTF-8 character to insert.
        PtrDiff index = reinterpret_cast<const char*>(ptr) - str + this->gap_idx_top;
        uint8_t width = (codepoint >= 0) ? static_cast<uint8_t>(utf8_width(codepoint)) : 0;

        // Insert the UTF-8 index and width at the position of the cursor.
        this->utf8idxs.insert(this->utf8idxs.begin() + this->count_left, index);
        this->utf8wids.insert(this->utf8wids.begin() + this->count_left, width);

        // Update the number of UTF-8 characters in the left side.
        this->count_left += 1;
    }

    // Update the top index of the gap.
    this->gap_idx_top += size;

}   // }}}

void GapBuffer::insert(StringView str)
{ this->insert(str.data(), str.size()); }

////////////////////////////////////////////////////////////////////////////////////////////
// GapBuffer: Cursor movement functions
////////////////////////////////////////////////////////////////////////////////////////////

void GapBuffer::move_cursor(PtrDiff delta)
{   // {{{

    // Clips the given "delta" within a movable range.
    delta = clip(delta, -this->count_left, this->count_right());

    // Do nothing if the delta is zero.
    if (delta == 0) return;

    // A variable to store the number of bytes to move.
    PtrDiff delta_bytes;

    // If the delta is positive, move the cursor to the right.
    if (delta > 0)
    {
        // Compute the number of bytes to move.
        if (delta == this->count_right())
            delta_bytes = this->size_right();
        else
            delta_bytes = this->utf8idxs[this->count_left + delta] - this->gap_idx_end;

        // Copy [gap_idx_end, gap_idx_end + delta_bytes) to [gap_idx_top, gap_idx_top + delta_bytes).
        std::move(this->buffer.begin() + this->gap_idx_end,
                  this->buffer.begin() + this->gap_idx_end + delta_bytes,
                  this->buffer.begin() + this->gap_idx_top);

        // Update the UTF-8 indices.
        for (PtrDiff idx = 0; idx < delta; ++idx)
            this->utf8idxs[this->count_left + idx] -= this->gap_size();
    }

    // If the delta is negative, move the cursor to the left.
    else
    {
        // Compute the number of bytes to move.
        delta_bytes = this->utf8idxs[this->count_left + delta] - this->gap_idx_top;

        // Copy [gap_idx_top + delta_bytes, gap_idx_top) to [gap_idx_end + delta_bytes, gap_idx_end).
        std::move_backward(this->buffer.begin() + this->gap_idx_top + delta_bytes,
                           this->buffer.begin() + this->gap_idx_top,
                           this->buffer.begin() + this->gap_idx_end);

        // Update the UTF-8 indices.
        for (PtrDiff idx = delta; idx < 0; ++idx)
            this->utf8idxs[this->count_left + idx] += this->gap_size();
    }

    // Update gap indices.
    this->gap_idx_top += delta_bytes;
    this->gap_idx_end += delta_bytes;

    // Update the number of UTF-8 characters in the left side.
    this->count_left += delta;

}   // }}}

void GapBuffer::move_top(void)
{ move_cursor(-this->count_left); }

void GapBuffer::move_end(void)
{ move_cursor(this->count_right()); }

////////////////////////////////////////////////////////////////////////////////////////////
// GapBuffer: Private functions
////////////////////////////////////////////////////////////////////////////////////////////

void GapBuffer::append_utf8_indices(StringView sv, PtrDiff offset)
{   // {{{

    for (const auto& [codepoint, ptr] : utf8_decode_iter(sv.data(), sv.size()))
    {
        // Calculate the index of the UTF-8 character to append, and append it.
        this->utf8idxs.emplace_back(reinterpret_cast<const char*>(ptr) - sv.data() + offset);

        // Calculate the width of the UTF-8 character to append, and append it.
        this->utf8wids.emplace_back((codepoint >= 0) ? static_cast<uint8_t>(utf8_width(codepoint)) : 0);
    }

}   // }}}

void GapBuffer::ensure_gap(SizeType need)
{   // {{{

    // Do nothing if the gap has enough size.
    if (this->gap_size() >= need) return;

    // Calculate the size of the buffer to reallocate, and memorize the previous buffer size.
    const SizeType new_size = max(buffer.size() * 2, buffer.size() + need);
    const SizeType old_size = this->buffer.size();

    // Calculate new gap indices.
    const SizeType new_gap_idx_end = new_size - this->size_right();

    // Resize the buffer to the new size.
    this->buffer.resize(new_size);

    // Move the right part of the gap buffer.
    // The function "move_backward" should be used to avoid destroying the buffer contents.
    std::move_backward(this->buffer.begin() + this->gap_idx_end, this->buffer.begin() + old_size, this->buffer.end());

    // Update the UTF-8 indices.
    for (SizeType idx = this->count_left; idx < this->utf8idxs.size(); ++idx)
        this->utf8idxs[idx] += new_gap_idx_end - this->gap_idx_end;

    // Update the gap index.
    this->gap_idx_end = new_gap_idx_end;

}   // }}}

SizeType GapBuffer::gap_size(void) const noexcept
{ return gap_idx_end - gap_idx_top; }

SizeType GapBuffer::size_right(void) const noexcept
{ return this->buffer.size() - gap_idx_end; }

PtrDiff GapBuffer::count_right(void) const noexcept
{ return this->count() - this->count_left; }

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
