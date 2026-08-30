////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: bash_completer.hxx                                                          ///
///                                                                                              ///
/// A class that provides shell-style tab completion by communicating with a bash process        ///
/// running in the background.                                                                   ///
///                                                                                              ///
/// The constructor spawns a child bash process and connects to its stdin/stdout via pipes.      ///
/// It then sources bash-completion and defines a helper function. Afterwards, complete() can be ///
/// called at any time to retrieve completion candidates for an arbitrary command-line string.   ///
///                                                                                              ///
/// Example usage:                                                                               ///
///                                                                                              ///
/// int main(void)                                                                               ///
/// {                                                                                            ///
///     BashCompleter bc;                                                                        ///
///                                                                                              ///
///     auto test = [&](StringView input)                                                        ///
///     {                                                                                        ///
///         std::cout << "Input: " << std::quoted(input) << "\n";                                ///
///         for (const auto& c : bc.complete(input))                                             ///
///             std::cout << "  " << c << "\n";                                                  ///
///         std::cout << std::endl;                                                              ///
///     };                                                                                       ///
///                                                                                              ///
///     test("git sta");   // Expected: stash, status, stage, etc.                               ///
///     test("git ");      // Expected: full list of git subcommands.                            ///
///     test("tar --");    // Expected: long options for tar.                                    ///
///     test("apt ");      // Expected: apt subcommands.                                         ///
///                                                                                              ///
///     return 0;                                                                                ///
/// }                                                                                            ///
///                                                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BASH_COMPLETER_HXX
#define BASH_COMPLETER_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////////////////////////////////

class BashCompleter
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

         BashCompleter(void); // Constructor: spawns a bash child process and initialises bash-completion.
        ~BashCompleter(void); // Destructor : closes the pipes and reaps the child process.

        // NOTE: This class should be non-copyable and non-movable, because this class owns raw file descriptors.
        BashCompleter(const BashCompleter&)            = delete;
        BashCompleter& operator=(const BashCompleter&) = delete;
        BashCompleter(BashCompleter&&)                 = delete;
        BashCompleter& operator=(BashCompleter&&)      = delete;

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        Vector<String> complete(StringView user_input);
        // Returns a list of completion candidates for the given command-line string.
        //
        // [Args]
        //   user_input (StringView): [IN] The partial command line typed by the user.
        //
        // [Returns]
        //   (Vector<String>): A vector of matching completion strings.

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        String read_until_sentinel(int timeout_ms = 5000);
        // Reads from the bash child process's stdout until a sentinel string is encountered
        // or a timeout occurs.
        //
        // [Args]
        //   timeout_ms (int): [IN] Timeout in milliseconds.
        //
        // [Returns]
        //   (String): The output read from the bash child process, excluding the sentinel string.

        void send_line(StringView line);
        // Sends a line of input to the bash child process's stdin.
        //
        // [Args]
        //   line (StringView): [IN] The line to send to bash.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private static functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        static String shell_quote(StringView sv);
        // Wraps a string in single quotes with proper escaping so it can be passed safely
        // to the shell as a single argument.
        //
        // [Args]
        //   sv (StringView): [IN] The string to be quoted.
        //
        // [Returns]
        //   (String): The quoted string.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        int write_fd = -1;
        // Write end of the pipe connected to bash's stdin.

        int read_fd  = -1 ;
        // Read end of the pipe connected to bash's stdout.

        pid_t pid = -1;
        // PID of the bash child process.

        static constexpr StringView SENTINEL = "___BASH_COMPLETER_DONE___";
        // A unique sentinel string used to detect the end of each command's output.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
