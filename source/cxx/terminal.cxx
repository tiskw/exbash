////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: terminal.cxx                                                                ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "terminal.hxx"

// Include STL headers.
#include <format>
#include <iostream>

// Include POSIX headers.
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>

// Include the headers of custom modules.
#include "utf8.hxx"
#include "string_utils.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    bool is_csis(const char* str, SizeType size)
    // Returns true if the given string starts from CSI (Control Sequence Introducer) sequence.
    //
    // [Args]
    //   str  (const char*): [IN] Target string.
    //   size (SizeType)   : [IN] Size of the given string.
    //
    // [Returns]
    //   (bool): True if the given string starts from an ANSI escape sequence.
    //
    {   // {{{

        return ((size >= 2) and (str[0] == '\x1B') and (str[1] == '['));

    }   // }}}

    uint8_t csis_byte_size(const char* str, SizeType size)
    // Returns the size of the given CSI (Control Sequence Introducer) sequence.
    // This function assumes that the given string starts from CSI sequence.
    //
    // [Args]
    //   str  (const char*): [IN] Target string.
    //   size (SizeType)   : [IN] Size of the given string.
    //
    // [Returns]
    //   (uint8_t): Byte size of the given ANSI escape sequence.
    //
    {   // {{{

        for (SizeType idx = 2; idx < size; ++idx)
            if ((('a' <= str[idx]) and (str[idx] <= 'z')) or (('A' <= str[idx]) and (str[idx] <= 'Z')))
                return static_cast<uint8_t>(idx + 1);

        return static_cast<uint8_t>(1);

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TermUserIF: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

TermUserIF::TermUserIF(uint16_t n_rows, uint16_t n_cols) : fd(STDIN_FILENO), read_bytes(0)
{   // {{{

    // Initialize the terminal size.
    this->term_size = {n_cols, n_rows};

    // Reserve space for the buffer to avoid frequent reallocations.
    this->buffer.reserve(1024);

    // Initialize the previous lines with empty strings.
    lines_prev.reserve(n_rows);
    for (uint16_t n = 0; n < n_rows; ++n)
        this->lines_prev.emplace_back("");

    // Initialize the terminal attributes for reading from STDIN.
    this->ini_terminal_attr();

    // Move the cursor up by n_rows and fill the area with newlines to clear it.
    this->buffer = String(n_rows - 1, '\n') + std::format("\x1B[{}F", n_rows - 1);
    this->print(this->buffer.data(), this->buffer.size());

    // Initialize tmpbuf and output buffers.
    std::memset(this->tmpbuf, '\0', sizeof(this->tmpbuf));
    std::memset(this->output, '\0', sizeof(this->output));

}   // }}}

TermUserIF::~TermUserIF(void)
{   // {{{

    // Restore the terminal attributes.
    this->fin_terminal_attr();

    // Clear the area to restore the terminal state.
    this->print("\x1B[0J", 4);

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TermUserIF: Member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

CharX TermUserIF::getch(int wakeup_fd)
{   // {{{

    // Read new characters from STDIN if no data remained in the buffer.
    if ((this->read_bytes == 0) or (this->read_bytes < utf8_byte_size(static_cast<uint8_t>(this->tmpbuf[0]))))
    {
        // When the buffer is empty, use select() to wait for stdin OR a completion wakeup.
        // This avoids the fixed 100ms VTIME polling lag when the worker finishes quickly.
        if (this->read_bytes == 0)
        {
            struct timeval tv = {1, 0};  // 1-second safety-net timeout.
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);

            int max_fd = STDIN_FILENO;
            if (wakeup_fd >= 0)
            {
                FD_SET(wakeup_fd, &fds);
                max_fd = std::max(max_fd, wakeup_fd);
            }

            const int ret = select(max_fd + 1, &fds, nullptr, nullptr, &tv);

            // On timeout or error, return empty character to trigger a re-render.
            if (ret <= 0)
                return CharX(nullptr, 0);

            // If the wakeup pipe fired, drain it to prevent spurious future wakeups.
            if (wakeup_fd >= 0 and FD_ISSET(wakeup_fd, &fds))
            {
                char drain_buf[64];
                while (read(wakeup_fd, drain_buf, sizeof(drain_buf)) > 0) {}

                // If stdin has no simultaneous input, return empty character to trigger
                // a re-render so the new completion result is picked up immediately.
                if (not FD_ISSET(STDIN_FILENO, &fds))
                    return CharX(nullptr, 0);
            }
        }

        // Read characters from STDIN.
        PtrDiff size = read(STDIN_FILENO, this->tmpbuf + this->read_bytes, this->tmpbuf_size - this->read_bytes - 1);

        if (size < 0)
            throw std::runtime_error("Failed to read from the terminal.");

        // Return empty character if timed out.
        if (size == 0)
            return CharX(nullptr, 0);

        // Update the number of bytes read from STDIN.
        this->read_bytes += size;
    }

    // Get the byte size of the first UTF-8 character in the buffer.
    uint8_t byte_size = is_csis(this->tmpbuf, this->read_bytes) ? csis_byte_size(this->tmpbuf, this->read_bytes)
                                                                : utf8_byte_size(static_cast<uint8_t>(tmpbuf[0]));

    // Copy the first UTF-8 character in the buffer to the output, then remove the character from the buffer.
    std::memset(this->output, '\0',         this->output_size);
    std::memcpy(this->output, this->tmpbuf, byte_size);
    std::memmove(this->tmpbuf, this->tmpbuf + byte_size, this->tmpbuf_size - byte_size);
    this->read_bytes -= byte_size;

    // Return the output character and its byte size.
    return CharX(this->output, byte_size);

}   // }}}

bool TermUserIF::update_lines(const Vector<String>& lines)
{   // {{{

    // Clear the buffer for the new update.
    this->buffer.clear();

    // Returns an error code if the number of lines is different from the previous update.
    if (lines.size() != this->lines_prev.size())
        return false;

    for (SizeType n = 0; n < lines.size(); ++n)
    {
        // If the current line is different from the previous line, update it.
        if (lines[n] != this->lines_prev[n])
        {
            // Move the cursor to the beginning of the line.
            this->buffer += lines[n] + "\x1B[0K";

            // Update the previous line with the current line.
            this->lines_prev[n] = lines[n];
        }

        // Move the cursor to the next line if it's not the last line.
        if (n != (lines.size() - 1))
            this->buffer += "\x1B[1E";
    }

    // Move the cursor to the beginning of the drawing area.
    this->buffer += std::format("\x1B[{}F", this->term_size.rows - 1);

    // Write the buffer to the terminal.
    this->print(buffer.c_str(), buffer.size());

    return true;

}   // }}}

void TermUserIF::update(StringView lhs, StringView rhs, StringView ps1, StringView ps2, const Vector<String>& clines,
                        StringView hist_comp, StringView histhint_pre, StringView histhint_post)
{   // {{{

    // Colorize the editing line.
    String edit_line;

    // If the right-hand side of the editing line is not empty, colorize the whole line.
    if (not rhs.empty())
    {
        edit_line += lhs;
        edit_line += rhs;
        edit_line  = colorize(edit_line);
    }

    // If the right-hand side of the editing line is empty, colorize only the left-hand side of the line.
    else
    {
        // Colorize the left-hand side of the editing line.
        edit_line = colorize(lhs);

        // If the history completion is not empty, append it to the editing line with the history hint.
        // The first character of the history completion is not included in the history hint colorization,
        // because the first character will be highlighted by the cursor (reverse effect).
        if (hist_comp.size() > 0)
        {
            edit_line += hist_comp.front();
            edit_line += histhint_pre;
            edit_line += hist_comp.substr(1);
            edit_line += histhint_post;
        }
    }

    // Calculate the cursor position in the editing line.
    String edit_line_cursor = insert_cursor(edit_line, width(lhs));

    // Compute the width of the prompt string.
    const uint16_t ps_width = max(width(ps1), width(ps2));

    // Create a vector of lines to be printed.
    Vector<String> lines;
    lines.reserve(this->term_size.rows);

    // Append editing lines.
    SizeType edit_line_chunk_size = 0;
    for (const StringView edit_line_chunk : chunk(edit_line_cursor, this->term_size.cols - ps_width))
        lines.emplace_back(std::format("{}{}\x1B[0K", (++edit_line_chunk_size == 1) ? ps1 : ps2, edit_line_chunk));

    // Compute the number of completion lines.
    const uint16_t n_clines = (this->term_size.rows > edit_line_chunk_size) ? this->term_size.rows - edit_line_chunk_size : 0;

    // Append completion lines.
    for (uint16_t n = 0; n < n_clines; ++n)
        if (n < clines.size())
            lines.emplace_back(clines[n] + "\x1B[0K");

    // Append empty lines if the number of lines is less than the number of rows.
    for (SizeType n = lines.size(); n < this->term_size.rows; ++n)
        lines.emplace_back("\x1B[0K");

    // Update the terminal with the new lines.
    this->update_lines(lines);

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TermUserIF: Private member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void TermUserIF::ini_terminal_attr(void)
{   // {{{

    // Copy current termios.
    if (tcgetattr(this->fd, &this->term) == -1)
        throw std::runtime_error("Failed to get terminal attributes.");

    // Create a copy of the current termios.
    struct termios term_cpy = this->term;

    // Change termios setting to cbreak mode.
    term_cpy.c_lflag &= ~(ICANON | ECHO);
    term_cpy.c_lflag &= ~ISIG;
    term_cpy.c_iflag &= ~ICRNL;
    term_cpy.c_cc[VMIN] = 0;
    term_cpy.c_cc[VTIME] = 0;  // Non-blocking: select() in getch() handles all waiting.

    // NOTE: termios.c_cc[VMIN] and termios.c_cc[VTIME].
    //       <https://manpages.debian.org/bookworm/manpages-dev/termios.3.en.html>
    // 
    // MIN == 0, TIME == 0 (polling read):
    //     If data is available, read(2) returns immediately, with the lesser of the number of bytes
    //     available, or the number of bytes requested. If no data is available, read(2) returns 0.
    //
    // MIN > 0, TIME == 0 (blocking read):
    //     read(2) blocks until MIN bytes are available, and returns up to the number of bytes
    //     requested.
    //
    // MIN == 0, TIME > 0 (read with timeout):
    //     TIME specifies the limit for a timer in tenths of a second. The timer is started when
    //     read(2) is called. read(2) returns either when at least one byte of data is available,
    //     or when the timer expires. If the timer expires without any input becoming available,
    //     read(2) returns 0. If data is already available at the time of the call to read(2),
    //     the call behaves as though the data was received immediately after the call.
    //
    // MIN > 0, TIME > 0 (read with interbyte timeout):
    //     TIME specifies the limit for a timer in tenths of a second. Once an initial byte of
    //     input becomes available, the timer is restarted after each further byte is received.
    //     read(2) returns when any of the following conditions is met:

    // Set the cbreak termios to STDIN.
    tcsetattr(this->fd, TCSAFLUSH, &term_cpy);

    // Hide cursor.
    this->print("\033[?25l", 6);

}   // }}}

void TermUserIF::fin_terminal_attr(void)
{   // {{{

    // Restore the termios saved in the constructor.
    tcsetattr(this->fd, TCSANOW, &this->term);

    // Show cursor and flush.
    this->print("\033[?25h", 6);

}   // }}}

void TermUserIF::print(const char* str, SizeType size)
{   // {{{

    // Write the given string to the terminal.
    PtrDiff ret = write(STDOUT_FILENO, str, size);

    // If the write system call returns an error, throw a runtime error.
    if (ret < 0)
        std::cerr << "Failed to write to the terminal." << std::endl;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
