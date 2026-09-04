////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: terminal.hxx                                                                ///
///                                                                                              ///
/// This file defines the class `TermUserIF` that manages user input and output       ///
/// in the terminal window.                                                                      ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TERMINAL_HXX
#define TERMINAL_HXX

// Include POSIX headers.
#include <termios.h>

// Include the headers of custom modules.
#include "char_x.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////////////////////////////////

class TermUserIF
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

         TermUserIF(uint16_t n_rows, uint16_t n_cols);
        ~TermUserIF(void);
        // Default constructor and destructor.
        //
        // [Args]
        //   n_rows (uint16_t): [IN] Number of rows in the terminal.
        //   n_cols (uint16_t): [IN] Number of columns in the terminal.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Public member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        CharX getch(int wakeup_fd = -1);
        // Get valid UTF-8 character from STDIN and returns it. If the acquired character is
        // registered in the keybind, convert the character to a binded string (most of the
        // binded string will be added to the stack).
        // If wakeup_fd >= 0, getch() also wakes up immediately when the fd becomes readable
        // (used to unblock the main loop as soon as a new async completion result is ready).
        //
        // [Args]
        //   wakeup_fd (int): [IN] Optional extra fd to watch (e.g. AsyncComp wakeup pipe). -1 to disable.
        //
        // [Returns]
        //   (const char*): Captured character.

        bool update_lines(const Vector<String>& lines);
        // Update the terminal with the given lines.
        //
        // [Args]
        //   lines (const Vector<String>&): [IN] Lines to be written to the terminal.
        //
        // [Returns]
        //   (bool): True if the update is successful, false otherwise.

        void update(StringView lhs, StringView rhs, StringView ps1, StringView ps2, const Vector<String>& clines,
                    StringView hist_comp, StringView histhint_pre, StringView histhint_post);

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        int fd;
        // File descriptor to read input characters.

        struct termios term;
        // Copy of the original termios. This class changes the termios, and this member
        // variable is used to save the original termios and restore when this class is deleted.

        Size term_size;
        // Number of columns and rows in the terminal.

        Vector<String> lines_prev;
        // Previous lines written to the terminal.
        // This is used to determine which lines need to be updated.

        String buffer;
        // Temporal buffer for constructing the output to be written to the terminal.

        static constexpr SizeType tmpbuf_size = 64;
        static constexpr SizeType output_size = 32;
        // Size of the "tmpbuf" and "output" buffers.

        char tmpbuf[tmpbuf_size] = {'\0'};
        char output[output_size] = {'\0'};
        // Buffer to read characters from STDIN.

        SizeType read_bytes;
        // The number of bytes in the "tmpbuf" read from STDIN.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void ini_terminal_attr(void);
        void fin_terminal_attr(void);
        // Initialize and finalize the terminal attributes.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private static member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        static void print(const char* str, SizeType size);
        // Write the given string to the terminal.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
