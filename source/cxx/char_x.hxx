////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: char_x.hxx                                                                  ///
///                                                                                              ///
/// A class to represent a UTF-8 character, with utility member functions.                       ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CHAR_X_HXX
#define CHAR_X_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////////////////////////////////

class CharX
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        CharX(const char* str, uint8_t byte_size);
        // Constructor with UTF-8 character and its size in bytes.
        //
        // [Args]
        //   str  (const char*): [IN] UTF-8 character string (must be at least 4 bytes long).
        //   size (uint8_t)    : [IN] Size of the UTF-8 character in bytes.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Getter functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        const char* c_str(void) const noexcept;
        // Get the pointer to the UTF-8 character string.
        //
        // [Returns]
        //   (const char*): Pointer to the UTF-8 character string.

        uint8_t size(void) const noexcept;
        // Get the size of the UTF-8 character in bytes.
        //
        // [Returns]
        //   (uint8_t): Size of the UTF-8 character in bytes.

        StringView view(void) const noexcept;
        // Get the string view of the UTF-8 character.
        //
        // [Returns]
        //   (StringView): String view of the UTF-8 character.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        StringView printable(void) noexcept;
        // Get the printable representation of the character.
        //
        // [Returns]
        //   (StringView): Printable representation of the character.

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        char buffer[8];
        // Buffer to store the UTF-8 character.

        uint8_t byte_size;
        // Size of the UTF-8 character in bytes.

        char buffer_printable[8];
        // Buffer to store the printable representation of the UTF-8 character.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
