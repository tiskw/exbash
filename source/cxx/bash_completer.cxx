////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: bash_completer.cpp                                                          ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "bash_completer.hxx"

// Include STL headers.
#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// Include POSIX headers.
#include <fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Bash initialisation script sent to the child process on startup.
//   - Sources the system-wide bash-completion library.
//   - Defines the __bc_complete helper function used by complete().
////////////////////////////////////////////////////////////////////////////////////////////////////

static const char INIT_SCRIPT[] = R"BASH_SCRIPT(
set +e
source /usr/share/bash-completion/bash_completion 2>/dev/null || true

# Completion helper function.
# Argument: the partial command-line string typed by the user (e.g. "git sta").
# Output:   one completion candidate per line written to stdout.
__bc_complete() {
    local input="$1"

    # Split the input into words, respecting shell quoting.
    # Fall back to simple whitespace splitting if eval fails.
    local -a words
    if ! eval "words=($input)" 2>/dev/null; then
        read -ra words <<< "$input"
    fi

    local nwords=${#words[@]}
    local cword

    # If the input ends with whitespace, the user is completing the next
    # (currently empty) word rather than extending the last one.
    if [[ "$input" =~ [[:space:]]$ ]]; then
        cword=$nwords
        words+=("")
    else
        cword=$(( nwords > 0 ? nwords - 1 : 0 ))
    fi

    # Populate the environment variables expected by bash-completion.
    COMP_LINE="$input"
    COMP_POINT="${#input}"
    COMP_WORDS=("${words[@]}")
    COMP_CWORD=$cword

    local cmd="${words[0]:-}"
    local cur="${words[$cword]:-}"
    local prev=""
    (( cword > 0 )) && prev="${words[$((cword - 1))]}"

    # Dynamically load the completion definition for the given command.
    if declare -f _completion_loader &>/dev/null; then
        _completion_loader "$cmd" 2>/dev/null || true
    fi

    COMPREPLY=()

    # Retrieve the completion spec and invoke the associated function.
    local compspec
    compspec=$(complete -p -- "$cmd" 2>/dev/null || true)

    if [[ "$compspec" =~ -F[[:space:]]+([^[:space:]]+) ]]; then
        local compfunc="${BASH_REMATCH[1]}"
        "$compfunc" "$cmd" "$cur" "$prev" 2>/dev/null || true
    elif [[ -n "$cur" ]]; then
        # Fall back to plain filename completion when no spec is found.
        mapfile -t COMPREPLY < <(compgen -f -- "$cur" 2>/dev/null)
    fi

    printf '%s\n' "${COMPREPLY[@]}"
}
)BASH_SCRIPT";

////////////////////////////////////////////////////////////////////////////////////////////////////
// BashCompleter: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

BashCompleter::BashCompleter(void)
{   // {{{

    // Create two pipes for communication with the child bash process.
    int to_bash[2], from_bash[2];
    if ((pipe(to_bash) < 0) or (pipe(from_bash) < 0))
        throw std::runtime_error(String("pipe() failed: ") + strerror(errno));

    // Fork a child process to run bash.
    this->pid = fork();
    if (this->pid < 0)
    {
        // Close all the pipes if fork() failed, to avoid resource leaks.
        close(to_bash[0]); close(to_bash[1]); close(from_bash[0]); close(from_bash[1]);

        throw std::runtime_error(String("fork() failed: ") + strerror(errno));
    }

    if (this->pid == 0)
    {
        // Child process: redirect the pipes to stdin and stdout.
        dup2(to_bash[0],   STDIN_FILENO);
        dup2(from_bash[1], STDOUT_FILENO);

        // Silence stderr so that errors from completion scripts stay hidden.
        const int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) { dup2(devnull, STDERR_FILENO); close(devnull); }

        // Close all the pipes if fork() failed, to avoid resource leaks.
        close(to_bash[0]); close(to_bash[1]); close(from_bash[0]); close(from_bash[1]);

        execlp("bash", "bash", "--norc", "--noprofile", nullptr);

        // If execlp() returns, it means the exec failed.
        _exit(EXIT_FAILURE);
    }

    // Parent process: close the unused ends of both pipes.
    close(to_bash[0]); close(from_bash[1]);
    this->write_fd = to_bash[1];
    this->read_fd  = from_bash[0];

    // Send the initialisation script and wait for the sentinel to confirm
    // that bash-completion has been loaded successfully.
    send_line(INIT_SCRIPT);
    send_line(String("echo ") + String(SENTINEL));
    read_until_sentinel(15000);  // Allow up to 15 seconds for the initial load.

}   // }}}

BashCompleter::~BashCompleter(void)
{   // {{{

    if (this->write_fd >= 0) { close(this->write_fd); this->write_fd = -1; }
    if (this->read_fd  >= 0) { close(this->read_fd);  this->read_fd  = -1; }
    if (this->pid > 0)       { waitpid(this->pid, nullptr, 0); this->pid = -1; }

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// BashCompleter: Member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void BashCompleter::send_line(StringView line)
{   // {{{

    String cmd{line};
    cmd += '\n';
    const char* ptr       = cmd.data();
    size_t      remaining = cmd.size();
    while (remaining > 0)
    {
        const ssize_t n = write(this->write_fd, ptr, remaining);
        if (n <= 0) break;
        ptr       += n;
        remaining -= static_cast<size_t>(n);
    }

}   // }}}

String BashCompleter::read_until_sentinel(int timeout_ms)
{   // {{{

    String buf;
    char tmp[4096];
    const String sentinel_line = String(SENTINEL) + "\n";

    while (true)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(this->read_fd, &fds);

        timeval tv{timeout_ms / 1000, (timeout_ms % 1000) * 1000};
        const int ret = select(this->read_fd + 1, &fds, nullptr, nullptr, &tv);
        if (ret <= 0) break;  // Timed out or encountered an error.

        const ssize_t n = read(this->read_fd, tmp, sizeof(tmp) - 1);
        if (n <= 0) break;
        tmp[n] = '\0';
        buf += tmp;

        const auto pos = buf.find(sentinel_line);
        if (pos != String::npos)
        {
            buf.resize(pos);
            break;
        }
    }

    return buf;

}   // }}}

Vector<String> BashCompleter::complete(StringView user_input)
{   // {{{

    // Send the user input to the bash child process and request completions.
    send_line("__bc_complete " + shell_quote(user_input));
    send_line(String("echo ") + String(SENTINEL));

    // Read the output from the bash child process until the sentinel is encountered.
    const String output = read_until_sentinel();

    // Split the output into lines and return them as a vector of strings.
    Vector<String> result;
    std::istringstream iss(output);
    String line;
    while (std::getline(iss, line))
        if (not line.empty())
            result.push_back(std::move(line));

    return result;

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// BashCompleter: Static functions
////////////////////////////////////////////////////////////////////////////////////////////////////

String BashCompleter::shell_quote(StringView sv)
{   // {{{

    // Wrap the string in single quotes and escape any embedded single quotes
    // using the '\'  idiom, making it safe to pass as a shell argument.
    String result = "'";
    for (const char c : sv)
    {
        if (c == '\'') { result += "'\\''"; }
        else           { result += c;       }
    }
    result += "'";
    return result;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
