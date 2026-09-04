////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: cmd_runner.cxx                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "cmd_runner.hxx"

// Include POSIX headers.
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

// When building with gcov (--coverage), flush coverage data from the child process before
// execvp() replaces the process image or _exit() discards it. The GCOV_BUILD macro is defined
// in the test Makefile's COVOPTS so this block is absent in normal production builds.
#ifdef GCOV_BUILD
    extern "C" void __gcov_dump(void);
    extern "C" void __gcov_reset(void);
    #define GCOV_DUMP()  __gcov_dump()
    #define GCOV_RESET() __gcov_reset()
#else
    #define GCOV_DUMP()  do {} while (0)
    #define GCOV_RESET() do {} while (0)
#endif

// Include the headers of custom modules.
#include "error.hxx"
#include "tokenizers.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

String run_command(StringView command, RunCommandOption option)
{   // {{{

    // Tokenize the command string and run the command.
    Vector<String> cmd_tokens;
    for (const StringView token : tokenize(command, TOKENIZE_DEQUOTE))
        cmd_tokens.emplace_back(token);

    return run_command(cmd_tokens, option);

}   // }}}

String run_command(const Vector<String>& cmd_tokens, RunCommandOption option)
{   // {{{

    // Parse the run_command option.
    const bool no_strip   = (option & RUN_COMMAND_NO_STRIP) != 0;
    const bool get_output = (option & RUN_COMMAND_GETOUT  ) != 0;

    // Return immediately if no command is given.
    if (cmd_tokens.empty()) return "";

    // The arguments for "execvp()" do not have the "const" keyword for historical reasons,
    // but the contents of the arguments will never change (this is guaranteed by POSIX).
    // The following "const_cast" is a workaround to avoid compilation errors
    // due to argument type errors in "execvp()" and is not dangerous.
    Vector<char*> argv;
    argv.reserve(cmd_tokens.size() + 1);
    for (const String& token : cmd_tokens)
        argv.emplace_back(const_cast<char*>(token.c_str()));
    argv.emplace_back(nullptr);

    // Create a pipe for inter-process communication if output capture is requested.
    int pipefd[2] = {-1, -1};
    if (get_output and pipe2(pipefd, O_CLOEXEC | O_NONBLOCK) == -1)
        return "";

    // Fork a child process to run the command.
    pid_t pid = fork();

    // Return an empty string if failed to fork.
    if (pid == -1)
    {
        // Get the error code and print the error message.
        print_error("Error", std::format("Failed to fork: {} (errno: {})", std::strerror(errno), errno));

        // Close the pipe file descriptors if the pipe was created.
        if (get_output) { close(pipefd[0]); close(pipefd[1]); }

        return "";
    }

    // Child process.
    if (pid == 0)
    {
        // Reset gcov counters so only child-process lines are tracked in this dump.
        // (The child inherits parent's in-memory counters from fork; resetting avoids
        //  double-counting those in the single GCOV_DUMP() call made before _exit)
        GCOV_RESET();

        if (get_output)
        {
            // Close the read end of the pipe in the child process.
            close(pipefd[0]);

            // Redirect the standard output and error to the write end of the pipe.
            if (dup2(pipefd[1], STDOUT_FILENO) < 0) { GCOV_DUMP(); _exit(EXIT_FAILURE); }
            if (dup2(pipefd[1], STDERR_FILENO) < 0) { GCOV_DUMP(); _exit(EXIT_FAILURE); }
            close(pipefd[1]);
        }

        // Restore the default signal handler for SIGINT in the child process, because the parent process
        // ignores SIGINT (in "main_exbash" function) but the child process should be able to receive SIGINT.
        if (signal(SIGINT, SIG_DFL) == SIG_ERR)
        {
            std::perror("Failed to register signal handler");
            GCOV_DUMP(); _exit(EXIT_FAILURE);
        }

        // Run the command.
        execvp(argv[0], argv.data());

        // If execvp returns, it means the command failed to run. Get the error code and print the error message.
        std::perror(std::format("Error ({}, L.{}): execvp failed: {}", __FILE__, __LINE__, argv[0]).c_str());

        // Flush coverage data before _exit() discards it. This is the single GCOV_DUMP call for
        // the child: placed here so it captures every line executed in the child (including
        // execvp above) when execvp fails.
        GCOV_DUMP(); _exit(EXIT_FAILURE);
    }

    // Parent process with no output capture.
    if (not get_output)
    {
        int32_t status;

        while (true)
        {
            // Get the child process status.
            const pid_t ret = waitpid(pid, &status, 0);

            // If waitpid() returns -1 and errno is EINTR, it means the wait
            // was interrupted by a signal, so we continue waiting.
            if ((ret == -1) and (errno == EINTR)) continue;

            // If waitpid() returns -1 and errno is not EINTR, it means an error occurred,
            // so we print the error message and return an empty string.
            if (ret == -1) { std::perror(std::format("Error ({}, L.{}): waitpid", __FILE__, __LINE__).c_str()); return ""; }

            // If the child process has exited or was terminated by a signal, we break the loop.
            if (WIFEXITED(status) or WIFSIGNALED(status)) break;
        }

        return "";
    }

    // Parent process with output capture.
    close(pipefd[1]);

    // Practical safety limit for completion / preview commands.
    // 1 MiB is usually plenty for completion candidates and file previews.
    constexpr SizeType max_output_size = 1024 * 1024;

    // Flags to track the child process and pipe status.
    bool child_done = false;
    bool pipe_eof   = false;
    bool killed     = false;

    // Variable to store the child process exit status.
    int32_t status = 0;

    // Buffer to read data from the pipe.
    char buffer[4096];

    // Initialize the output string to store the captured output from the child process.
    String output;

    while (not (child_done and pipe_eof))
    {
        // Step 1: Drain pipe as much as possible.
        while (not pipe_eof)
        {
            // Read from the pipe in a non-blocking manner.
            const SignedSizeType n_read = read(pipefd[0], buffer, sizeof(buffer));

            // Case 1-1: Successfully read data from the pipe.
            if (n_read > 0)
            {
                if (output.size() < max_output_size)
                {
                    const SizeType remaining = max_output_size - output.size();
                    const SizeType n_append  = min(static_cast<SizeType>(n_read), remaining);
                    output.append(buffer, n_append);
                }

                // If too much output is produced, kill the child.
                if ((output.size() >= max_output_size) and (not child_done) and (not killed))
                { kill(pid, SIGTERM); killed = true; }

                continue;
            }

            // Case 1-2: End of file (EOF) reached, the child process has closed the pipe.
            if (n_read == 0)
            {
                pipe_eof = true;
                break;
            }

            // Case 1-3: An error occurred while reading from the pipe (n_read < 0).
            if ((errno == EAGAIN) or (errno == EWOULDBLOCK)) break;

            // Case 1-4: An error occurred while reading from the pipe (n_read < 0) and it was not EAGAIN or EWOULDBLOCK.
            if (errno == EINTR) continue;

            // Case 1-5: An error occurred while reading from the pipe (n_read < 0) and it was not EAGAIN, EWOULDBLOCK, or EINTR.
            std::perror("read"); pipe_eof = true; break;
        }

        // Step 2: Check child status without blocking.
        if (not child_done)
        {
            // Check the child process status without blocking.
            const pid_t ret = waitpid(pid, &status, WNOHANG);

            // Case 2-1: The child process has exited or was terminated by a signal.
            if (ret == pid)
            {
                child_done = true;
            }

            // Case 2-2: The child process has not changed state yet (WNOHANG).
            else if (ret == -1)
            {
                // If waitpid() returns -1 and errno is EINTR, it means the wait
                // was interrupted by a signal, so we continue waiting.
                if (errno == EINTR) continue;

                // If waitpid() returns -1 and errno is not EINTR, it means an error occurred,
                // so we print the error message and set child_done to true to avoid further
                // waitpid calls.
                std::perror("waitpid"); child_done = true;
            }
        }

        // Step 3: Avoid busy loop by waiting for either the pipe to be readable or a timeout.
        if (not (child_done and pipe_eof))
        {
            // Create a timeval structure for the timeout.
            struct timeval tv;
            tv.tv_sec  = 0;
            tv.tv_usec = 10000;  // 10 ms

            // Create a file descriptor set for select().
            fd_set rfds;
            FD_ZERO(&rfds);
            if (not pipe_eof)
                FD_SET(pipefd[0], &rfds);

            // Wait for either the pipe to be readable or the timeout to occur.
            const int ret = select(pipefd[0] + 1, &rfds, nullptr, nullptr, &tv);

            // If "select()" returns -1 and errno is not EINTR, print an error message and break the loop.
            if ((ret < 0) and (errno != EINTR))
            { std::perror("select"); break; }
        }
    }

    close(pipefd[0]);

    // If SIGTERM was not sufficient, reap has already happened in normal cases.
    // If the child somehow still exists, waitpid() below handles it safely.
    if (not child_done)
    {
        while (true)
        {
            // Wait for the child process to change state.
            const pid_t ret = waitpid(pid, &status, 0);
            if ((ret == -1) and (errno == EINTR))
                continue;

            break;
        }
    }

    // Returns the output, optionally stripping leading and trailing whitespace.
    return no_strip ? output : String(strip(output));

}   // }}}

String run_command(const Vector<StringView>& cmd_tokens, RunCommandOption option)
{   // {{{

    // The "StringView" is not compatible with "execvp()", because "execvp()" requires
    // null-terminated strings, but StringView does not guarantee null-termination. Therefore
    // convert the command tokens to String before running the command.

    Vector<String> cmd_tokens_str;
    for (const StringView token_view : cmd_tokens)
        cmd_tokens_str.emplace_back(token_view);

    return run_command(cmd_tokens_str, option);

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
