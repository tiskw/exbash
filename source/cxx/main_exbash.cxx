////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: main_exbash.cxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the headers of custom modules.
#include "main_exbash.hxx"

// Include STL headers.
#include <fstream>
#include <future>
#include <iostream>

// Include POSIX headers.
#include <signal.h>
#include <unistd.h>

// Include the header of the cxxopts library.
#include <cxxopts.hpp>

// Include the headers of custom modules.
#include "cmd_runner.hxx"
#include "config.hxx"
#include "dtypes.hxx"
#include "error.hxx"
#include "gen_path_cache.hxx"
#include "logo.hxx"
#include "read_cmd.hxx"
#include "string_utils.hxx"
#include "tokenizers.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    String get_git_branch_info(void)
    // Get git branch and status information and return as a colored string.
    // 
    // [Returns]
    //   (String): Colored string of git information.
    //
    {   // {{{

        // Returns empty string if not a Git directory.
        if (not stdfs::exists(".git"))
            return "";

        // Get the branch name and its status at the same time.
        // NOTE: the following is an example of the command output:
        //
        // $ git status --porcelain=v2 --branch
        // # branch.oid 53bd39614a7c66c6a3816a1c5c1e528db4c35c52
        // # branch.head alpha
        // # branch.upstream origin/alpha
        // # branch.ab +1 -0
        // 1 .M N... 100644 100644 100644 ca6703623a3e5897f3b183d5a998d6791bd3bd15 ca6703623a3e5897f3b183d5a998d6791bd3bd15 source/cxx/main_exbash.cxx
        //
        const String git_status = run_command("git status --porcelain=v2 --branch", RUN_COMMAND_GETOUT);

        // Initialize the git branch name and the changed flag.
        String branch     = "???";
        bool   is_changed = false;
    
        for (const StringView sv : split(git_status, "\n"))
        {
            // Get the branch name.
            if (sv.starts_with("# branch.head "))
                branch = sv.substr(14);

            // Set the changed flag if non-branch line is dumped.
            else if (not sv.starts_with("# "))
                is_changed = true;
        }

        // Colorize as yellow if the git status is "changed".
        if (!branch.empty() and is_changed)
            return "\x1B[38;2;235;193;111m" + branch + "!\x1B[m";

        // Colorize as green if the git status is "unchanged".
        if (!branch.empty())
            return "\x1B[38;2;181;189;104m" + branch +  "\x1B[m";

        return branch;

    }   // }}}

    void print_ps0(String ps0l, String ps0r, StringView hline_color, StringView hline_char)
    // Print the prompt string 0.
    //
    // [Args]
    //   ps0l        (String)    : [IN] Left side of the prompt string 0.
    //   ps0r        (String)    : [IN] Right side of the prompt string 0.
    //   hline_color (StringView): [IN] Color code of the horizontal line. If empty, the horizontal line will not be printed.
    //   hline_char  (StringView): [IN] Character for the horizontal line.
    //
    // [Notes]
    //   This function supports simple replacement of variables.
    //
    {   // {{{

        // Start to compute git info, because this process takes time.
        std::future<String> future_git_info;
        if (ps0r.find("{git}") != String::npos)
            future_git_info = launch_async(get_git_branch_info);

        // Get terminal size.
        const Size term_size = get_terminal_size();

        // Print the horizontal line.
        if (hline_color.size() > 0)
        {
            // Create the horizontal line.
            String hline = String(hline_color);
            hline.reserve(hline_color.size() + term_size.cols * hline_char.size());
            for (int c = 0; c < term_size.cols; ++c)
                hline.append(hline_char);

            // Print the horizontal line.
            std::cout << hline << "\x1B[0m" << '\n';
        }

        // Do not print ps0 if empty.
        if (ps0l.empty() and ps0r.empty())
            return;

        // Reserve temporary buffer.
        constexpr SizeType buffer_size = 512;
        char buffer[buffer_size];

        // Get the current time.
        time_t raw_time = std::time(nullptr);

        // Replace basic variables.
        if (ps0l.find("{user}") != String::npos and (getlogin_r(buffer, buffer_size) == 0))
            ps0l = replace(ps0l, "{user}", buffer);
        if (ps0l.find("{host}") != String::npos and (gethostname(buffer, buffer_size) == 0))
            ps0l = replace(ps0l, "{host}", buffer);
        if (ps0l.find("{date}") != String::npos)
            ps0l = replace(ps0l, "{date}", get_time(raw_time, "%Y/%m/%d"));
        if (ps0l.find("{time}") != String::npos)
            ps0l = replace(ps0l, "{time}", get_time(raw_time, "%H:%M:%S"));
        if ((ps0l.find("{cwd}") != String::npos) and (getcwd(buffer, buffer_size) != nullptr))
            ps0l = replace(ps0l, "{cwd}", buffer);
        if (ps0l.find("{empty}") != String::npos)
            ps0l = replace(ps0l, "{empty}", "");

        // Replace environmetal variables.
        while (true)
        {
            // Get the location of the open curly brackets.
            const String::size_type pos1 = ps0l.find("{");
            if (pos1 == String::npos)
                break;

            // Get the location of the close curly brackets.
            const String::size_type pos2 = ps0l.find("}", pos1 + 1);
            if (pos2 == String::npos)
                break;

            // Get the replace target and variable name.
            const String target = ps0l.substr(pos1,     pos2 - pos1 + 1);
            const String envvar = ps0l.substr(pos1 + 1, pos2 - pos1 - 1);
            const char*  envval = getenv(envvar.c_str());

            // Replace the target with the environment variable value if exists, otherwise replace with empty string.
            ps0l = replace(ps0l, target, envval ? envval : "");
        }

        // Replace the "{git}" variable with the git info.
        if (ps0r.find("{git}") != String::npos)
            ps0r = replace(ps0r, "{git}", future_git_info.get());

        // Append whitespaces to the left side of the ps0.
        SizeType width_ps0 = width(ps0l) + width(ps0r);
        if (width_ps0 < term_size.cols)
            ps0l += String(term_size.cols - width_ps0, ' ');

        // Print ps0.
        std::cout << ps0l << ps0r << std::endl;

    }   // }}}

    void print_user_input(StringView user_input, StringView datetime_pre, StringView datetime_post)
    // Print the user input (and time stamp).
    //
    // [Args]
    //   user_input    (StringView): [IN] User input.
    //   datetime_pre  (StringView): [IN] String to be printed before the datetime stamp.
    //   datetime_post (StringView): [IN] String to be printed after the datetime stamp.
    //
    {   // {{{

        // Clear the prompt 0 (move the cursor up and clear from the cursor to the end of line).
        std::cout << "\x1B[1F\x1B[0K";

        // Print datetime stamp.
        if (!datetime_pre.empty() or !datetime_post.empty())
            std::cout << datetime_pre << get_time(std::time(nullptr), "%Y/%m/%d %H:%M:%S") << datetime_post;

        // Print the user input.
        std::cout << colorize(user_input) << '\n';

    }   // }}}

    Deque<String> read_history(StringView path_hist, uint16_t max_hist_size)
    // Read history file and return history entries.
    //
    // [Args]
    //   path_hist     (StringView): [IN] Path to history file.
    //   max_hist_size (uint16_t)  : [IN] Maximum number of history entries to be stored in the output queue.
    //
    // [Returns]
    //   (Deque<String>): A collection of history lines.
    //
    {   // {{{

        // Initialize the output queue.
        Deque<String> queue;

        // Open the history file.
        std::ifstream ifp(expand_tilde(path_hist));
        if (not ifp.is_open())
            return queue;

        // Create temporary string data.
        String line;

        // Read the history file.
        while (getline(ifp, line))
        {

            // Remove one string from the front if the queue size is too large.
            if (queue.size() == max_hist_size)
                queue.pop_front();

            // Strip the line.
            line = strip(line);

            // Append if the line is not empty.
            if (line.size() > 0)
                queue.emplace_back(line);
        }

        return queue;

    }   // }}}

    Tuple<String, String> run_keybind(const ReadCmdOut& rc_out, const StringMap& keybinds, const String& output_plugin)
    // Run the given keybind.
    //
    // [Args]
    //   rc_out        (const ReadCmdOut&): [IN] Output of "readcmd" function.
    //   keybinds      (const StringMap&) : [IN] Map of keybinds in the config file.
    //   output_plugin (const String&)    : [IN] Path to the plugin output file.
    //
    // [Returns]
    //   (Tuple<String, String>): Left and right hand side of the editing buffer after keybind.
    //
    {   // {{{

        // If the given key is not registered, do nothing.
        if (not keybinds.contains(rc_out.stop))
            return {rc_out.lhs, rc_out.rhs};

        // Create a map for placeholders replacement.
        const StringMap extra = {
            {"{lhs}",           rc_out.lhs},
            {"{rhs}",           rc_out.rhs},
            {"{output_plugin}", expand_tilde(output_plugin)},
        };

        // Tokenize the command string with placeholder replacement.
        Vector<String> cmd_tokens;
        for (const String& token : tokenize_with_placeholder_replacement(keybinds.at(rc_out.stop), extra, TOKENIZE_DEQUOTE))
            cmd_tokens.emplace_back(token);

        // Run the tokenized command.
        run_command(cmd_tokens);

        // Read the output of the command from the command output file.
        std::ifstream ifs(output_plugin);
        if (!ifs)
        {
            std::cerr << "Failed to open file: " << output_plugin << std::endl;
            return {rc_out.lhs, rc_out.rhs};
        }

        // Get the left and right hand side of the editing buffer from the command output file.
        String lhs_new, rhs_new;
        std::getline(ifs, lhs_new);
        std::getline(ifs, rhs_new);

        return {lhs_new, rhs_new};

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main function
////////////////////////////////////////////////////////////////////////////////////////////////////

int32_t main_exbash(int32_t argc, char* argv[], const char* input_str)
{   // {{{

    if (signal(SIGINT, SIG_IGN) == SIG_ERR)
        return print_error("Error", "Failed to register signal handler");

    // Parse command line arguments.
    cxxopts::Options options(SOFTWARE_NAME, SOFTWARE_DESC);
    options.add_options()
        ("c,config", "Path to config file.",           cxxopts::value<String>())
        ("e,editor", "Editor mode ('emacs' or 'vi').", cxxopts::value<String>())
        ("o,output", "Path to output file.",           cxxopts::value<String>())
        ("r,run",    "Run other task and exit.",       cxxopts::value<String>())
        ("h,help",   "Show this help message and exit.");

    // Parse the arguments and print help message if -h/--help is given.
    const auto args = options.parse(argc, argv);
    if (args.count("help"))
    {
        std::cout << "Usage:"                                                  << '\n';
        std::cout << "    exbash [OPTION...]"                                  << '\n';
        std::cout << ""                                                        << '\n';
        std::cout << "Lightweight alternative to GNU Readline for ExBash."     << '\n';
        std::cout << ""                                                        << '\n';
        std::cout << "Behavioral options:"                                     << '\n';
        std::cout << "    -c, --config PATH  Path to config file."             << '\n';
        std::cout << "    -e, --editor STR   Editor mode ('emacs' or 'vi')."   << '\n';
        std::cout << "    -o, --output PATH  Path to output file."             << '\n';
        std::cout << "    -r, --run STR      Run other task and exit."         << '\n';
        std::cout << ""                                                        << '\n';
        std::cout << "Other options:"                                          << '\n';
        std::cout << "    -h, --help         Show this help message and exit." << '\n';
        return EXIT_SUCCESS;
    }

    // Load the config values.
    const ExBashConfig cfg = load_config(args.count("config") ? args["config"].as<String>() : "");

    // Run other task and exit if -r/--run is specified.
    if (args.count("run"))
    {
        // Case 1: --run generate_path_commands_cache
        if (args["run"].as<String>() == "generate_path_commands_cache")
            return generate_path_commands_cache(cfg);

        // Case 2: --run logo
        if (args["run"].as<String>() == "logo")
            return print_exbash_logo();

        // Case 3: --run print_config=...
        if (args["run"].as<String>().starts_with("print_config"))
            return print_config(cfg, args["run"].as<String>());

        // Otherwise: unknown --run command.
        return print_error("Error", "Unknown run command: " + args["run"].as<String>());
    }

    // Read the history file.
    Deque<String> hists = read_history(cfg.path_history, cfg.max_hist_size);

    // Get the editor mode.
    const String editor_name = args.count("editor") ? args["editor"].as<String>() : "emacs";

    // Print the prompt 0.
    print_ps0(cfg.ps0l, cfg.ps0r, cfg.hline_color, cfg.hline_char);

    // Initialize text buffer.
    ReadCmdOut rc_out = {"", "", "", (input_str != nullptr) ? StringView(input_str) : StringView("")};

    // Start user editing loop.
    while (true)
    {
        // Get user input.
        rc_out = readcmd(rc_out.lhs, rc_out.rhs, hists, editor_name, rc_out.input, cfg);

        // Exit from the while loop if the user editing stopped without stop key.
        if (rc_out.stop.size() == 0)
            break;

        // Otherwise, run keybind command of the stop key, and continue the loop.
        std::tie(rc_out.lhs, rc_out.rhs) = run_keybind(rc_out, cfg.keybinds, cfg.output_plugin);
    }

    // Compute user input string.
    String user_input;
    user_input += rc_out.lhs;
    user_input += rc_out.rhs;

    // Print the user input to STDOUT.
    print_user_input(user_input, cfg.datetime_pre, cfg.datetime_post);

    // Write the user input to the output file if specified.
    if (args.count("output"))
    {
        // Open the output file.
        std::ofstream ofs(args["output"].as<String>());
        if (not ofs.is_open())
        {
            std::cerr << "Failed to open file: " << args["output"].as<String>() << std::endl;
            return EXIT_FAILURE;
        }

        // Write the user input to the output file.
        ofs << user_input << '\n';

        // Close the file.
        ofs.close();
    }

    return EXIT_SUCCESS;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
