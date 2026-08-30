////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: char_x.cxx                                                                  ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "char_x.hxx"

// Include STL headers.
#include <cstring>

// Include the headers of custom modules.
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CharX: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

CharX::CharX(const char* str, uint8_t byte_size) : byte_size(0)
{   // {{{

    // Initialize the buffer with null characters.
    std::memset(this->buffer, '\0', sizeof(this->buffer));

    // Initialize the printable buffer with null characters.
    std::memset(this->buffer_printable, '\0', sizeof(this->buffer_printable));

    if ((str != nullptr) and (byte_size != 0))
    {
        // Safeguard: Limit the byte size to the maximum size of the buffer.
        this->byte_size = min(byte_size, static_cast<uint8_t>(sizeof(this->buffer) - 1));

        // Copy the input string to the buffer and null-terminate it.
        std::memcpy(this->buffer, str, this->byte_size);
    }

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// CharX: Getter functions
////////////////////////////////////////////////////////////////////////////////////////////////////

const char* CharX::c_str(void) const noexcept
{ return this->buffer; }

uint8_t CharX::size(void) const noexcept
{ return this->byte_size; }

StringView CharX::view(void) const noexcept
{ return StringView(this->buffer, this->byte_size); }

////////////////////////////////////////////////////////////////////////////////////////////////////
// CharX: Member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

StringView CharX::printable(void) noexcept
{   // {{{

    // Skip the creation of the printable string if it is already created.
    if (this->buffer_printable[0] != '\0')
        return StringView(this->buffer_printable);

    // Case 0: Empty character. Return an empty string.
    if (this->byte_size == 0) return StringView("");

    // Case 1: Multi-byte character. Return the character itself as a printable string.
    if (this->byte_size > 1)
        std::memcpy(this->buffer_printable, this->buffer, this->byte_size);

    // Case 2: Control character. Return the caret notation of the control character.
    else if (this->buffer[0] <= 0x1F)
    {
        this->buffer_printable[0] = '^';
        this->buffer_printable[1] = static_cast<char>(0x40 + this->buffer[0]);
        this->buffer_printable[2] = '\0';
    }

    // Case 3: Delete character. Return "^?" as the printable string.
    else if (this->buffer[0] == 0x7F)
        std::memcpy(this->buffer_printable, "^?", 3);

    // Case 4: Other 1 byte character. Return the character itself
    else
        this->buffer_printable[0] = this->buffer[0];

    return StringView(this->buffer_printable);

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
