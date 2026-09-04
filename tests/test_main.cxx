////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: test_main.cxx                                                               ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the headers of STL.
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>

// Include POSIX headers.
#include <fcntl.h>
#include <pty.h>
#include <unistd.h>

// Include the headers of custom modules.
#include "async_comp.hxx"
#include "bash_completer.hxx"
#include "edit_helper.hxx"
#include "char_x.hxx"
#include "cmd_runner.hxx"
#include "config.hxx"
#include "dtypes.hxx"
#include "gap_buffer.hxx"
#include "gen_path_cache.hxx"
#include "history_manager.hxx"
#include "logo.hxx"
#include "main_exbash.hxx"
#include "mime_type.hxx"
#include "path_x.hxx"
#include "preview.hxx"
#include "read_cmd.hxx"
#include "string_utils.hxx"
#include "terminal.hxx"
#include "text_editor_emacs.hxx"
#include "text_editor_vi.hxx"
#include "tokenizers.hxx"
#include "utf8.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////////////////////////////////

static bool passed = true;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility macros and functions for testing
////////////////////////////////////////////////////////////////////////////////////////////////////

#define assert(expr) (assert_body((expr), #expr, __LINE__))

static void assert_body(bool expr_bool, const char* expr_str, int line_no)
{   // {{{

    // Print test result.
    if (expr_bool) std::cout << "\033[32mPASSED\033[0m";
    else           std::cout << "\033[31mFAILED\033[0m";

    // Print detailed information.
    std::cout << ": L." << line_no << ": " << expr_str << std::endl;

    // Update the passed/failed flag.
    passed &= expr_bool;

}   // }}}

static void print_header(const char* message)
{   // {{{

    std::cout                                       << std::endl;
    std::cout << "------------------------------"   << std::endl;
    std::cout << "\033[33m" << message << "\033[0m" << std::endl;
    std::cout                                       << std::endl;

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Unittest functions
////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_BashCompleter(void)
{   // {{{

    // Print header.
    print_header("Unit test for BashCompleter class");

    ////////////////////////////////////////////////////////
    // Constructor and destructor
    ////////////////////////////////////////////////////////

    {
        // Constructing BashCompleter spawns a bash child process.
        // Simply constructing and destroying must not throw or crash.
        BashCompleter bc;
    }

    ////////////////////////////////////////////////////////
    // complete: git subcommands with prefix
    ////////////////////////////////////////////////////////

    {
        BashCompleter bc;

        // "git sta" should produce completions containing "status" and/or "stash".
        // Note: bash-completion may append a trailing space to each candidate.
        const Vector<String> results = bc.complete("git sta");
        bool found_status = false;
        bool found_stash  = false;
        for (const String& s : results)
        {
            // Match with or without trailing space.
            if (s == "status" || s == "status ") found_status = true;
            if (s == "stash"  || s == "stash " ) found_stash  = true;
        }
        assert(found_status || found_stash);
    }

    ////////////////////////////////////////////////////////
    // complete: trailing space returns non-empty list
    ////////////////////////////////////////////////////////

    {
        BashCompleter bc;

        // "git " (with trailing space) should return the full list of git subcommands.
        const Vector<String> results = bc.complete("git ");
        assert(!results.empty());
    }

    ////////////////////////////////////////////////////////
    // complete: empty input does not crash
    ////////////////////////////////////////////////////////

    {
        BashCompleter bc;

        // An empty string is a degenerate input; complete() must not throw or crash.
        [[maybe_unused]] const Vector<String> results = bc.complete("");
    }

    ////////////////////////////////////////////////////////
    // complete: multiple sequential calls on the same object
    ////////////////////////////////////////////////////////

    {
        BashCompleter bc;

        // The same BashCompleter instance must handle multiple calls correctly.
        [[maybe_unused]] const Vector<String> r1 = bc.complete("ls --");
        [[maybe_unused]] const Vector<String> r2 = bc.complete("git ");
        [[maybe_unused]] const Vector<String> r3 = bc.complete("echo ");

        // Each call must return a Vector (possibly empty) without crashing.
        assert(true);
    }

    ////////////////////////////////////////////////////////
    // complete: input with single-quote character (shell quoting)
    ////////////////////////////////////////////////////////

    {
        BashCompleter bc;

        // Input that contains a single-quote must be shell-quoted correctly and
        // must not cause the child bash process to hang or crash.
        [[maybe_unused]] const Vector<String> results = bc.complete("echo 'hello");
    }

    ////////////////////////////////////////////////////////
    // complete: "tar --" returns option completions
    ////////////////////////////////////////////////////////

    {
        BashCompleter bc;

        // "tar --" should return at least one long option such as "--create" or "--help".
        const Vector<String> results = bc.complete("tar --");
        bool found_option = false;
        for (const String& s : results)
            if (s.size() >= 2 && s[0] == '-' && s[1] == '-') { found_option = true; break; }
        assert(found_option);
    }

}   // }}}

static void test_AsyncComp(void)
{   // {{{

    // Print header.
    print_header("Unit test for AsyncComp class");

    // Load the test configuration.
    const ExBashConfig cfg = load_config("misc/config.toml");

    ////////////////////////////////////////////////////////
    // Constructor, get_wakeup_fd, and destructor
    ////////////////////////////////////////////////////////

    {
        // Constructing AsyncComp launches the worker thread and creates the wakeup pipe.
        AsyncComp ac(8, 80, cfg);

        // The read end of the wakeup pipe must be a valid file descriptor.
        assert(ac.get_wakeup_fd() >= 0);

        // When no task has been processed yet, get_completion_result returns the cached
        // (initially empty) result without blocking.
        [[maybe_unused]] const Vector<String> r0 = ac.get_completion_result();

        // Destructor is called here: signals the worker to stop and joins the thread.
    }

    ////////////////////////////////////////////////////////
    // launch_async_completion and get_completion_result
    ////////////////////////////////////////////////////////

    {
        AsyncComp ac(8, 80, cfg);

        // Post a completion task for a simple command prefix.
        ac.launch_async_completion("echo ");

        // Wait for the worker thread to finish processing.
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        // After the worker finishes, get_completion_result returns the computed result.
        const Vector<String> r1 = ac.get_completion_result();
        (void)r1;

        // A second call returns the cached result (has_result is now false).
        const Vector<String> r2 = ac.get_completion_result();
        (void)r2;

        // Launching the same input again is a no-op (deduplication by lhs_result).
        ac.launch_async_completion("echo ");

        // Launching different inputs in quick succession: only the last one should be processed.
        ac.launch_async_completion("ls ");
        ac.launch_async_completion("git ");

        // Retrieve the result before the worker finishes (returns cached r2).
        const Vector<String> r3 = ac.get_completion_result();
        (void)r3;

        // Wait and retrieve the final result.
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        const Vector<String> r4 = ac.get_completion_result();
        (void)r4;
    }

    ////////////////////////////////////////////////////////
    // Empty string input
    ////////////////////////////////////////////////////////

    {
        AsyncComp ac(8, 80, cfg);
        ac.launch_async_completion("");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        [[maybe_unused]] const Vector<String> result = ac.get_completion_result();
    }

}   // }}}

static void test_CharX(void)
{   // {{{

    // Print header.
    print_header("Unit test for CharX class");

    ////////////////////////////////////////////////////////
    // ASCII single-byte character
    ////////////////////////////////////////////////////////

    // Construct from a single ASCII byte and verify basic accessors.
    CharX cx_ascii("A", 1);
    assert(cx_ascii.size() == 1);
    assert(std::strcmp(cx_ascii.c_str(), "A") == 0);
    assert(cx_ascii.view() == "A");

    // Printable representation of a normal ASCII character is itself.
    assert(cx_ascii.printable() == "A");

    ////////////////////////////////////////////////////////
    // Multi-byte UTF-8 character
    ////////////////////////////////////////////////////////

    // "あ" is U+3042, encoded in 3 bytes: 0xE3 0x81 0x82.
    const char* hiragana_a = "あ";
    CharX cx_mb(hiragana_a, 3);
    assert(cx_mb.size() == 3);
    assert(cx_mb.view() == "あ");

    // Multi-byte characters are printed as-is.
    assert(cx_mb.printable() == "あ");

    ////////////////////////////////////////////////////////
    // Control character (caret notation)
    ////////////////////////////////////////////////////////

    // Ctrl-A (0x01) should be printed as "^A".
    CharX cx_ctrl("\x01", 1);
    assert(cx_ctrl.size() == 1);
    assert(cx_ctrl.printable() == "^A");

    // Ctrl-C (0x03) should be printed as "^C".
    CharX cx_ctrl_c("\x03", 1);
    assert(cx_ctrl_c.printable() == "^C");

    ////////////////////////////////////////////////////////
    // DEL character (0x7F)
    ////////////////////////////////////////////////////////

    // DEL (0x7F) should be printed as "^?".
    CharX cx_del("\x7F", 1);
    assert(cx_del.size() == 1);
    assert(cx_del.printable() == "^?");

    ////////////////////////////////////////////////////////
    // Null / zero-size character
    ////////////////////////////////////////////////////////

    // A CharX with a null pointer or zero byte size should yield empty printable string.
    CharX cx_null(nullptr, 0);
    assert(cx_null.size() == 0);
    assert(cx_null.printable() == "");

}   // }}}

static void test_CmdRunner(void)
{   // {{{

    // Print header.
    print_header("Unit test for cmd_runner.cxx");

    ////////////////////////////////////////////////////////
    // run_command(StringView): capture output
    ////////////////////////////////////////////////////////

    {
        // A simple echo command returns its argument as a stripped string.
        const String result = run_command("echo hello", RUN_COMMAND_GETOUT);
        assert(result == "hello");
    }

    ////////////////////////////////////////////////////////
    // run_command(Vector<String>): capture output
    ////////////////////////////////////////////////////////

    {
        // The Vector<String> overload passes each token as a separate argument.
        const Vector<String> cmd = {"echo", "world"};
        const String result = run_command(cmd, RUN_COMMAND_GETOUT);
        assert(result == "world");
    }

    ////////////////////////////////////////////////////////
    // run_command(Vector<StringView>): capture output
    ////////////////////////////////////////////////////////

    {
        // The Vector<StringView> overload converts tokens to String before exec.
        const Vector<StringView> cmd = {"echo", "test"};
        const String result = run_command(cmd, RUN_COMMAND_GETOUT);
        assert(result == "test");
    }

    ////////////////////////////////////////////////////////
    // RUN_COMMAND_NO_STRIP: preserve trailing newline
    ////////////////////////////////////////////////////////

    {
        // Without stripping, echo's trailing newline is preserved.
        const RunCommandOption opt =
            static_cast<RunCommandOption>(RUN_COMMAND_GETOUT | RUN_COMMAND_NO_STRIP);
        const String result = run_command("echo hello", opt);
        assert(result == "hello\n");
    }

    ////////////////////////////////////////////////////////
    // RUN_COMMAND_PLAIN: fire-and-forget without capturing output
    ////////////////////////////////////////////////////////

    {
        // A plain run always returns an empty string.
        const String result = run_command("true", RUN_COMMAND_PLAIN);
        assert(result == "");
    }

    ////////////////////////////////////////////////////////
    // WIFSIGNALED: child process killed by a signal
    ////////////////////////////////////////////////////////

    {
        // "bash -c 'kill -9 $$'" sends SIGKILL to the bash subprocess itself.
        // run_command must handle the WIFSIGNALED exit status gracefully.
        const Vector<String> cmd = {"bash", "-c", "kill -9 $$"};
        run_command(cmd, RUN_COMMAND_GETOUT);
        run_command(cmd, RUN_COMMAND_PLAIN);
    }

    ////////////////////////////////////////////////////////
    // Non-existent command: execvp fails in the child process
    ////////////////////////////////////////////////////////

    {
        // When the command cannot be found, the child calls _exit() after execvp fails.
        // The parent should still get a well-defined exit status.
        const Vector<String> bad_cmd = {"__nonexistent_command_xyz_test__"};
        run_command(bad_cmd, RUN_COMMAND_GETOUT);
        run_command(bad_cmd, RUN_COMMAND_PLAIN);
    }

}   // }}}

static void test_ExBashConfig(void)
{   // {{{

    // Exercise the config loader with a broken, typo-containing, and valid config file.
    // Mainly ensures no crash occurs even with malformed input.
    load_config("misc/config_broken.toml");
    load_config("misc/config_typo.toml");
    const ExBashConfig cfg = load_config("misc/config.toml");

    // Print one of the config paths as a basic smoke test.
    print_config(cfg, "print_config=path_init_info");

}   // }}}

static void test_GapBuffer(void)
{   // {{{

    // Print header.
    print_header("Unit test for GapBuffer class");

    ////////////////////////////////////////////////////////
    // Getter
    ////////////////////////////////////////////////////////

    GapBuffer gap_buffer("this is a ペン, ", "that is an りんご.");
    assert(gap_buffer.count() == 29);
    assert(gap_buffer.cursor() == 14);
    assert(gap_buffer.lhs_view() == "this is a ペン, ");
    assert(gap_buffer.rhs_view() == "that is an りんご.");
    assert(gap_buffer.serialize() == "this is a ペン, that is an りんご.");
    assert(gap_buffer.size() == 39);

    ////////////////////////////////////////////////////////
    // Move cursor
    ////////////////////////////////////////////////////////

    gap_buffer.set("Here is Tokyo, ", "ここは東京。");
    gap_buffer.move_cursor(-3);
    assert(gap_buffer.lhs_view() == "Here is Toky");
    assert(gap_buffer.rhs_view() == "o, ここは東京。");
    gap_buffer.move_cursor(+6);
    assert(gap_buffer.lhs_view() == "Here is Tokyo, ここは");
    assert(gap_buffer.rhs_view() == "東京。");

    gap_buffer.set("Here is Tokyo, ", "ここは東京。");
    gap_buffer.move_top();
    std::cout << gap_buffer.lhs_view() << "/" << gap_buffer.rhs_view() << std::endl;
    assert(gap_buffer.lhs_view() == "");
    assert(gap_buffer.rhs_view() == "Here is Tokyo, ここは東京。");
    gap_buffer.move_end();
    std::cout << gap_buffer.lhs_view() << "/" << gap_buffer.rhs_view() << std::endl;
    assert(gap_buffer.lhs_view() == "Here is Tokyo, ここは東京。");
    assert(gap_buffer.rhs_view() == "");

    ////////////////////////////////////////////////////////
    // Backspace and deletekey
    ////////////////////////////////////////////////////////

    // Backspace.
    gap_buffer.set("aあbいcうdえeお", "<END>");
    gap_buffer.backspace(1);
    assert(gap_buffer.serialize() == "aあbいcうdえe<END>");
    gap_buffer.backspace(2);
    assert(gap_buffer.serialize() == "aあbいcうd<END>");
    gap_buffer.backspace(3);
    assert(gap_buffer.serialize() == "aあbい<END>");
    gap_buffer.backspace(4);
    assert(gap_buffer.serialize() == "<END>");

    // Deletekey.
    gap_buffer.set("<START>", "aあbいcうdえeお");
    gap_buffer.deletekey(2);
    assert(gap_buffer.serialize() == "<START>bいcうdえeお");
    gap_buffer.deletekey(3);
    assert(gap_buffer.serialize() == "<START>うdえeお");
    gap_buffer.deletekey(4);
    assert(gap_buffer.serialize() == "<START>お");

    // Large number (clamp at buffer size).
    gap_buffer.set("Here is Tokyo, ", "ここは東京。");
    gap_buffer.backspace(100);
    gap_buffer.deletekey(100);
    assert(gap_buffer.lhs_view() == "");
    assert(gap_buffer.rhs_view() == "");

    ////////////////////////////////////////////////////////
    // Insert
    ////////////////////////////////////////////////////////

    gap_buffer.set("Here is Tokyo, ", "ここは東京。");
    gap_buffer.insert("comfortable city, ");
    assert(gap_buffer.lhs_view() == "Here is Tokyo, comfortable city, ");

    gap_buffer.set("Here is Tokyo, ", "ここは東京。");
    gap_buffer.insert(String("comfortable city, "));
    assert(gap_buffer.lhs_view() == "Here is Tokyo, comfortable city, ");

    ////////////////////////////////////////////////////////
    // Erase
    ////////////////////////////////////////////////////////

    gap_buffer.set("Here is Tokyo, ", "ここは東京。");
    gap_buffer.erase_lhs();
    assert(gap_buffer.lhs_view() == "");

    gap_buffer.set("Here is Tokyo, ", "ここは東京。");
    gap_buffer.erase_rhs();
    assert(gap_buffer.rhs_view() == "");

    // Erase all characters.
    gap_buffer.set("Hello", "World");
    gap_buffer.erase();
    assert(gap_buffer.lhs_view() == "");
    assert(gap_buffer.rhs_view() == "");
    assert(gap_buffer.count() == 0);

    ////////////////////////////////////////////////////////
    // Ensure gap (capacity growth)
    ////////////////////////////////////////////////////////

    gap_buffer.insert(String(10000, 'a'));
    std::cout << "gap_buffer.capacity() = " << gap_buffer.capacity() << std::endl;

    ////////////////////////////////////////////////////////
    // Count and cursor tracking
    ////////////////////////////////////////////////////////

    // After inserting ASCII characters, count reflects the character count.
    gap_buffer.set("abc", "xyz");
    assert(gap_buffer.count() == 6);
    assert(gap_buffer.cursor() == 3);

    // Moving the cursor should update the cursor position.
    gap_buffer.move_cursor(-2);
    assert(gap_buffer.cursor() == 1);
    gap_buffer.move_cursor(+10);  // Clamps at end.
    assert(gap_buffer.cursor() == 6);

    ////////////////////////////////////////////////////////
    // Mixed ASCII and multibyte content
    ////////////////////////////////////////////////////////

    // Verify that count reflects character count, not byte count.
    gap_buffer.set("日本語", "");
    assert(gap_buffer.count() == 3);
    assert(gap_buffer.size() == 9);  // 3 characters * 3 bytes each.

}   // }}}

static void test_GenPathCache(void)
{   // {{{

    // Print header.
    print_header("Unit test for gen_path_cache.cxx");

    ////////////////////////////////////////////////////////
    // Generate the PATH-command cache from the test configuration
    ////////////////////////////////////////////////////////

    // Load the test configuration, which points to a writable cache path.
    const ExBashConfig cfg = load_config("misc/config.toml");

    // Generating the cache should succeed (PATH is expected to be set in the test environment).
    const int32_t ret = generate_path_commands_cache(cfg);
    assert(ret == EXIT_SUCCESS);

}   // }}}

static void test_HistManager(void)
{   // {{{

    // Print header.
    print_header("Unit test for HistManager class");

    ////////////////////////////////////////////////////////
    // Basic history completion
    ////////////////////////////////////////////////////////

    const Deque<String> hists = {"git status", "git diff", "ls -la", "git commit -m 'fix'"};
    HistManager hm(hists);

    // Prefix "git" matches the most recent entry starting with "git" ("git commit -m 'fix'").
    StringView result = hm.complete("git");
    assert(result == " commit -m 'fix'");

    // More specific prefix "git d" matches "git diff".
    result = hm.complete("git d");
    assert(result == "iff");

    // Exact match returns the suffix (empty, since lhs is not included in result).
    result = hm.complete("ls -la");
    assert(result == "");

    ////////////////////////////////////////////////////////
    // No matching history
    ////////////////////////////////////////////////////////

    // A prefix that matches nothing should return an empty string.
    result = hm.complete("docker");
    assert(result == "");

    ////////////////////////////////////////////////////////
    // Empty query
    ////////////////////////////////////////////////////////

    // An empty prefix should return an empty string (no meaningful completion).
    result = hm.complete("");
    assert(result == "");

    ////////////////////////////////////////////////////////
    // Empty history
    ////////////////////////////////////////////////////////

    const Deque<String> empty_hists;
    HistManager hm_empty(empty_hists);
    result = hm_empty.complete("ls");
    assert(result == "");

    ////////////////////////////////////////////////////////
    // Most recent match wins (reverse search)
    ////////////////////////////////////////////////////////

    const Deque<String> hists2 = {"ls /tmp", "ls /home", "ls /usr"};
    HistManager hm2(hists2);

    // "ls " matches the most recently added entry "ls /usr".
    result = hm2.complete("ls ");
    assert(result == "/usr");

}   // }}}

static void test_MimeType(void)
{   // {{{

    // Print header.
    print_header("Unit test for MimeType class");

    MimeType mime;

    ////////////////////////////////////////////////////////
    // File extension based MIME type lookup
    ////////////////////////////////////////////////////////

    // Common text/source file extensions.
    assert(mime.get("file.txt")  != "");
    assert(mime.get("file.html") != "");
    assert(mime.get("file.py")   != "");

    // Unknown/no extension falls back to text/plain.
    assert(mime.get("Makefile") == "text/plain");
    assert(mime.get("noextension") == "text/plain");

    ////////////////////////////////////////////////////////
    // Directory
    ////////////////////////////////////////////////////////

    // Existing directory should be identified as inode/directory.
    assert(mime.get(".") == "inode/directory");

    ////////////////////////////////////////////////////////
    // Image / binary types
    ////////////////////////////////////////////////////////

    // PNG and JPEG are well-known MIME types.
    const String png_mime  = mime.get("image.png");
    const String jpeg_mime = mime.get("photo.jpg");
    assert(png_mime.find("image") != String::npos or png_mime == "text/plain");
    assert(jpeg_mime.find("image") != String::npos or jpeg_mime == "text/plain");

}   // }}}

static void test_PathX(void)
{   // {{{

    // Print header.
    print_header("Unit test for PathX class");

    // Test 1: parent path.
    assert(PathX("~/workspace/Makefile").parent_path() == PathX("~/workspace"));
    assert(PathX("~/workspace/Makefile").parent_path() != PathX("~/workspace/"));
    assert(PathX("").parent_path() == PathX(""));

    // Test 2: split_to_target_and_query (no target).
    Vector<StringView> tokens1;
    tokens1.push_back("ls");
    tokens1.push_back(" ");
    auto [path1, name1] = split_to_target_and_query(tokens1);
    assert(path1 == PathX(""));
    assert(name1 == "");

    // Test 3: split_to_target_and_query (target is a file).
    Vector<StringView> tokens2;
    tokens2.push_back("ls");
    tokens2.push_back(" ");
    tokens2.push_back("../develop/nishiki");
    auto [path2, name2] = split_to_target_and_query(tokens2);
    assert(path2 == PathX("../develop"));
    assert(name2 == "nishiki");

    // Test 4: split_to_target_and_query (target is a directory).
    std::vector<StringView> tokens3;
    tokens3.push_back("ls");
    tokens3.push_back(" ");
    tokens3.push_back("../develop/nishiki/");
    auto [path3, name3] = split_to_target_and_query(tokens3);
    assert(path3 == PathX("../develop/nishiki"));
    assert(name3 == "");

    // Test 5: listdir.
    assert(PathX("/not_exists").listdir().size() == 0);
    assert(PathX(".").listdir(1).size() == 1);

}   // }}}

static void test_preview(void)
{   // {{{

    // Print header.
    print_header("Unit test for preview function");

    // An empty previews map is passed to use the default preview behavior.
    StringMap previews;

    // Test 1: preview non-existing file returns empty result.
    assert(preview("/unexisting_file", 100, previews).size() == 0);

    // Test 2: preview of an existing text file returns non-empty result.
    assert(preview("test_main.cxx", 100, previews).size() > 0);

}   // }}}

static void test_string_utils(void)
{   // {{{

    // Print header.
    print_header("Unit test for string_utils.cxx");

    ////////////////////////////////////////////////////////
    // colorize: basic output (visual check, limited iterations)
    ////////////////////////////////////////////////////////

    const char* str = "echo 'Hello, World!' | grep -i 'hello'";
    std::cout << str << std::endl;
    std::cout << colorize(str) << std::endl;

    // Check strange quote.
    const char* str2 = "echo 'Hello'echo'Hello'if'Hello'|'Hello'";
    std::cout << str2 << std::endl;
    std::cout << colorize(str2) << std::endl;

    // Spot-check a few cursor positions to verify insert_cursor/colorize work together.
    for (int i = 0; i <= 5; ++i)
        std::cout << insert_cursor(colorize(str), i) << std::endl;

    ////////////////////////////////////////////////////////
    // width: ASCII and multibyte strings
    ////////////////////////////////////////////////////////

    // ASCII string: each character has display width 1.
    assert(width("") == 0);
    assert(width("abc") == 3);
    assert(width("Hello") == 5);

    // Japanese characters have display width 2 each.
    assert(width("あ") == 2);
    assert(width("日本語") == 6);

    // Mixed ASCII and Japanese.
    assert(width("aあ") == 3);

    ////////////////////////////////////////////////////////
    // textclip: clip string to given width
    ////////////////////////////////////////////////////////

    // Clipping an ASCII string.
    assert(textclip("Hello", 3) == "Hel");
    assert(textclip("Hello", 10) == "Hello");

    // Width of 0 means no clipping.
    assert(textclip("Hello", 0) == "Hello");

    // Clipping at a boundary that falls within a multibyte character.
    // "あい" has width 4; clipping to 3 should yield only "あ".
    assert(textclip("あい", 3) == "あ");

    // Clipping exactly at the end of a multibyte character.
    assert(textclip("あい", 4) == "あい");

    ////////////////////////////////////////////////////////
    // chunk: split string into width-based chunks
    ////////////////////////////////////////////////////////

    // Chunk an ASCII string of width 5 into chunks of width 2.
    {
        uint32_t count = 0;
        for (const StringView sv : chunk("Hello", 2))
        {
            switch (count++)
            {
                case 0: assert(sv == "He"); break;
                case 1: assert(sv == "ll"); break;
                case 2: assert(sv == "o");  break;
                default: assert(false);
            }
        }
        assert(count == 3);
    }

    // Chunk of width 0 yields the full string as a single chunk.
    {
        uint32_t count = 0;
        for (const StringView sv : chunk("Hello", 0))
        {
            assert(sv == "Hello");
            ++count;
        }
        assert(count == 1);
    }

    // Chunk with multibyte characters: "日本語" has width 6; chunk at width 4 gives "日本" then "語".
    {
        uint32_t count = 0;
        for (const StringView sv : chunk("日本語", 4))
        {
            switch (count++)
            {
                case 0: assert(sv == "日本"); break;
                case 1: assert(sv == "語");   break;
                default: assert(false);
            }
        }
        assert(count == 2);
    }

}   // }}}

static void test_TermUserIF_pty(void)
{   // {{{

    // Print header.
    print_header("Unit test for TermUserIF with PTY");

    // Open a pseudo-terminal pair.  The slave end is a real TTY device, so TermUserIF can be
    // constructed and tested without needing an interactive terminal attached to the process.
    int master_fd, slave_fd;
    if (openpty(&master_fd, &slave_fd, nullptr, nullptr, nullptr) < 0)
    {
        std::cerr << "openpty() failed — skipping PTY-based TermUserIF tests" << std::endl;
        return;
    }

    // Replace stdin with the PTY slave (TermUserIF uses STDIN_FILENO for tcgetattr and read).
    // Replace stdout with /dev/null to suppress terminal escape sequences during the test.
    const int saved_stdin  = dup(STDIN_FILENO);
    const int saved_stdout = dup(STDOUT_FILENO);
    const int devnull_fd   = open("/dev/null", O_WRONLY);
    dup2(slave_fd, STDIN_FILENO);
    dup2(devnull_fd, STDOUT_FILENO);

    try
    {
        ////////////////////////////////////////////////////////
        // getch(): simple ASCII character
        // Exercises the select → read → return path of getch().
        ////////////////////////////////////////////////////////

        {
            TermUserIF term(8, 80);

            // Write 'A' to the master end; the PTY slave (our STDIN) immediately has data.
            write(master_fd, "A", 1);
            CharX cx = term.getch(-1);
            assert(cx.size() == 1);
            assert(cx.c_str()[0] == 'A');
        }

        ////////////////////////////////////////////////////////
        // getch(): complete CSI escape sequence "ESC [ A"
        // Exercises is_csis() (returns true) and csis_byte_size() success path (lines 58-60).
        ////////////////////////////////////////////////////////

        {
            TermUserIF term(8, 80);

            write(master_fd, "\x1B[A", 3);  // cursor-up CSI sequence
            CharX cx = term.getch(-1);
            assert(cx.size() == 3);
            assert(cx.c_str()[0] == '\x1B');
        }

        ////////////////////////////////////////////////////////
        // getch(): incomplete CSI sequence "ESC [ 1" (no letter terminator)
        // Exercises csis_byte_size() fallback return (line 62): the for-loop finds no
        // letter in positions [2, size), so the function returns 1 (just the ESC byte).
        ////////////////////////////////////////////////////////

        {
            TermUserIF term(8, 80);

            write(master_fd, "\x1B[1", 3);  // CSI without a letter terminator
            CharX cx = term.getch(-1);
            assert(cx.size() == 1);
            assert(cx.c_str()[0] == '\x1B');
        }

        ////////////////////////////////////////////////////////
        // update(): padding empty lines (line 278)
        // When clines is empty, the padding loop fills the remaining rows with blank lines.
        ////////////////////////////////////////////////////////

        {
            TermUserIF term(8, 80);
            term.update("hello", "", "$ ", "  ", {}, "", "", "");
        }

        ////////////////////////////////////////////////////////
        // update(): non-empty rhs branch (lines 230-232)
        // When rhs is non-empty, update() colorizes the whole line (lhs+rhs).
        ////////////////////////////////////////////////////////

        {
            TermUserIF term(8, 80);
            term.update("hello ", "world", "$ ", "  ", {}, "", "", "");
        }

        ////////////////////////////////////////////////////////
        // update(): hist_comp branch (lines 246-249)
        // When hist_comp is non-empty, update() appends the completion hint.
        ////////////////////////////////////////////////////////

        {
            TermUserIF term(8, 80);
            term.update("ls", "", "$ ", "  ", {}, " -la", "\x1B[2m", "\x1B[0m");
        }

        ////////////////////////////////////////////////////////
        // update(): with non-empty clines (line 274)
        // When clines has entries, they are appended after the editing line.
        ////////////////////////////////////////////////////////

        {
            TermUserIF term(8, 80);
            const Vector<String> clines = {"completion1", "completion2"};
            term.update("ls", "", "$ ", "  ", clines, "", "", "");
        }

        ////////////////////////////////////////////////////////
        // getch() with wakeup_fd set but only stdin fires (lines 131-132)
        // Passing a valid wakeup_fd exercises the FD_SET(wakeup_fd) branch.
        ////////////////////////////////////////////////////////

        {
            TermUserIF term(8, 80);
            int pipefd[2];
            pipe2(pipefd, O_NONBLOCK);  // non-blocking: mirrors how AsyncComp creates its pipe
            // Write to STDIN only; pipefd[0] (wakeup_fd) has no data.
            write(master_fd, "D", 1);
            CharX cx = term.getch(pipefd[0]);
            assert(cx.size() == 1);
            assert(cx.c_str()[0] == 'D');
            close(pipefd[0]);
            close(pipefd[1]);
        }

        ////////////////////////////////////////////////////////
        // getch() with wakeup_fd fires but no stdin data (lines 145-150)
        // When only the wakeup pipe fires, getch() drains it and returns empty.
        ////////////////////////////////////////////////////////

        {
            TermUserIF term(8, 80);
            int pipefd[2];
            pipe2(pipefd, O_NONBLOCK);  // non-blocking: required so the drain loop terminates
            // Write to the wakeup pipe only; STDIN has no data.
            write(pipefd[1], "w", 1);
            CharX cx = term.getch(pipefd[0]);
            assert(cx.size() == 0);  // empty CharX signals re-render
            close(pipefd[0]);
            close(pipefd[1]);
        }

        ////////////////////////////////////////////////////////
        // update_lines(): wrong line count returns false (line 191)
        ////////////////////////////////////////////////////////

        {
            TermUserIF term(8, 80);
            const Vector<String> wrong = {"only one line"};
            assert(term.update_lines(wrong) == false);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "PTY test exception: " << e.what() << std::endl;
    }

    // Restore stdin and stdout.
    dup2(saved_stdin,  STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdin);
    close(saved_stdout);
    close(master_fd);
    close(slave_fd);
    close(devnull_fd);

}   // }}}

static void test_TextEditorEmacs(void)
{   // {{{

    // Print header.
    print_header("Unit test for TextEditorEmacs class");

    ////////////////////////////////////////////////////////
    // Basic character insertion
    ////////////////////////////////////////////////////////

    {
        // ASCII characters are inserted at the cursor position.
        TextEditorEmacs e("", "", {});
        e.edit("h"); e.edit("i");
        assert(e.get_lhs() == "hi");
        assert(e.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // Cursor movement: C-a (BOL) and C-e (EOL)
    ////////////////////////////////////////////////////////

    {
        // C-a moves the cursor to the beginning of the line.
        TextEditorEmacs e("hello", "", {});
        e.edit("\x01");
        assert(e.get_lhs() == "");
        assert(e.get_rhs() == "hello");
    }

    {
        // C-e moves the cursor to the end of the line.
        TextEditorEmacs e("", "hello", {});
        e.edit("\x05");
        assert(e.get_lhs() == "hello");
        assert(e.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // Cursor movement: C-b (backward) and C-f (forward)
    ////////////////////////////////////////////////////////

    {
        // C-b moves one character backward.
        TextEditorEmacs e("abc", "", {});
        e.edit("\x02");
        assert(e.get_lhs() == "ab");
        assert(e.get_rhs() == "c");
    }

    {
        // C-f moves one character forward.
        TextEditorEmacs e("", "abc", {});
        e.edit("\x06");
        assert(e.get_lhs() == "a");
        assert(e.get_rhs() == "bc");
    }

    ////////////////////////////////////////////////////////
    // Deletion: C-d (forward), C-h / DEL (backward)
    ////////////////////////////////////////////////////////

    {
        // C-d deletes the character under the cursor.
        TextEditorEmacs e("", "hello", {});
        e.edit("\x04");
        assert(e.get_lhs() == "");
        assert(e.get_rhs() == "ello");
    }

    {
        // C-h (Backspace) deletes one character backward.
        TextEditorEmacs e("abc", "", {});
        e.edit("\x08");
        assert(e.get_lhs() == "ab");
    }

    {
        // DEL (0x7F) also deletes one character backward.
        TextEditorEmacs e("abc", "", {});
        e.edit("\x7F");
        assert(e.get_lhs() == "ab");
    }

    ////////////////////////////////////////////////////////
    // Kill commands: C-k (kill to EOL) and C-u (kill to BOL)
    ////////////////////////////////////////////////////////

    {
        // C-k kills from the cursor to the end of the line.
        TextEditorEmacs e("ls ", "/tmp", {});
        e.edit("\x0B");
        assert(e.get_lhs() == "ls ");
        assert(e.get_rhs() == "");
    }

    {
        // C-u kills from the beginning of the line to the cursor.
        TextEditorEmacs e("ls /tmp", "", {});
        e.edit("\x15");
        assert(e.get_lhs() == "");
        assert(e.get_rhs() == "");
    }

    {
        // C-y yanks the most recently killed text back into the buffer.
        TextEditorEmacs e("hello world", "", {});
        e.edit("\x15");  // C-u: kill everything → kill_ring = "hello world"
        e.edit("\x19");  // C-y: paste
        assert(e.get_lhs() == "hello world");
    }

    ////////////////////////////////////////////////////////
    // C-w: kill backward word (whitespace-delimited)
    ////////////////////////////////////////////////////////

    {
        // C-w kills the word immediately before the cursor, including the preceding space.
        TextEditorEmacs e("ls /tmp", "", {});
        e.edit("\x17");
        assert(e.get_lhs() == "ls");
        assert(e.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // C-t: transpose characters
    ////////////////////////////////////////////////////////

    {
        // C-t swaps the character before the cursor with the one at the cursor.
        TextEditorEmacs e("ab", "c", {});
        e.edit("\x14");
        assert(e.get_lhs() == "acb");
        assert(e.get_rhs() == "");
    }

    {
        // At end of line C-t swaps the last two characters.
        TextEditorEmacs e("ab", "", {});
        e.edit("\x14");
        assert(e.get_lhs() == "ba");
        assert(e.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // History navigation: C-p (previous) and C-n (next)
    ////////////////////////////////////////////////////////

    {
        Deque<String> hists = {"cmd1", "cmd2"};
        TextEditorEmacs e("", "", hists);
        e.edit("\x10");  // C-p: switch to most-recent history entry "cmd2".
        assert(e.get_lhs() == "cmd2");
        e.edit("\x10");  // C-p: switch to older entry "cmd1".
        assert(e.get_lhs() == "cmd1");
        e.edit("\x0E");  // C-n: switch back toward current buffer ("cmd2").
        assert(e.get_lhs() == "cmd2");
    }

    ////////////////////////////////////////////////////////
    // Arrow key sequences (3-byte CSI escape sequences)
    ////////////////////////////////////////////////////////

    {
        Deque<String> hists = {"old"};
        TextEditorEmacs e("abc", "", hists);

        // Up arrow navigates to the previous history entry.
        e.edit(StringView("\x1B[A", 3));
        assert(e.get_lhs() == "old");

        // Down arrow returns to the current editing buffer.
        e.edit(StringView("\x1B[B", 3));
        assert(e.get_lhs() == "abc");

        // Left arrow moves the cursor one character backward.
        e.edit(StringView("\x1B[D", 3));
        assert(e.get_lhs() == "ab");
        assert(e.get_rhs() == "c");

        // Right arrow moves the cursor one character forward.
        e.edit(StringView("\x1B[C", 3));
        assert(e.get_lhs() == "abc");
        assert(e.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // Meta keys (M-f, M-b): word forward and backward
    ////////////////////////////////////////////////////////

    {
        // M-f advances the cursor to the end of the next word.
        TextEditorEmacs e("", "hello world", {});
        e.edit("\x1B");  // ESC: arm the meta prefix
        e.edit("f");     // M-f: advance past "hello"
        assert(e.get_lhs() == "hello");
        assert(e.get_rhs() == " world");
    }

    {
        // M-b retreats the cursor to the start of the previous word.
        TextEditorEmacs e("hello world", "", {});
        e.edit("\x1B");
        e.edit("b");     // M-b: retreat past "world"
        assert(e.get_lhs() == "hello ");
        assert(e.get_rhs() == "world");
    }

    ////////////////////////////////////////////////////////
    // Meta keys (M-d, M-DEL): kill word forward and backward
    ////////////////////////////////////////////////////////

    {
        // M-d kills from the cursor to the end of the next word.
        TextEditorEmacs e("", "hello world", {});
        e.edit("\x1B");
        e.edit("d");     // M-d: kill "hello"
        assert(e.get_lhs() == "");
        assert(e.get_rhs() == " world");
    }

    {
        // M-DEL kills from the cursor back to the start of the previous word.
        TextEditorEmacs e("hello world", "", {});
        e.edit("\x1B");
        e.edit("\x7F");  // M-DEL: kill "world"
        assert(e.get_lhs() == "hello ");
        assert(e.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // Meta keys (M-u/l/c): word case transformation
    ////////////////////////////////////////////////////////

    {
        // M-u uppercases the next word.
        TextEditorEmacs eu("", "hello", {});
        eu.edit("\x1B");
        eu.edit("u");
        assert(eu.get_lhs() == "HELLO");
    }

    {
        // M-l lowercases the next word.
        TextEditorEmacs el("", "HELLO", {});
        el.edit("\x1B");
        el.edit("l");
        assert(el.get_lhs() == "hello");
    }

    {
        // M-c capitalizes the next word.
        TextEditorEmacs ec("", "hello", {});
        ec.edit("\x1B");
        ec.edit("c");
        assert(ec.get_lhs() == "Hello");
    }

    ////////////////////////////////////////////////////////
    // Double ESC (M-ESC) and unrecognized Meta key
    ////////////////////////////////////////////////////////

    {
        // M-ESC (double ESC) is a no-op; the buffer is not modified.
        TextEditorEmacs e("hi", "", {});
        e.edit("\x1B");
        e.edit("\x1B");
        assert(e.get_lhs() == "hi");
    }

    {
        // An unrecognized Meta key inserts the character as-is.
        TextEditorEmacs e("", "", {});
        e.edit("\x1B");
        e.edit("z");  // M-z is unrecognized; 'z' is inserted.
        assert(e.get_lhs() == "z");
    }

    ////////////////////////////////////////////////////////
    // Control characters that are silently ignored
    ////////////////////////////////////////////////////////

    {
        // C-c is not handled by the editor and must not modify the buffer.
        TextEditorEmacs e("", "", {});
        e.edit("\x03");
        assert(e.get_lhs() == "");
    }

}   // }}}

static void test_TextEditorVi(void)
{   // {{{

    // Print header.
    print_header("Unit test for TextEditorVi class");

    ////////////////////////////////////////////////////////
    // INSERT mode: default state and character insertion
    ////////////////////////////////////////////////////////

    {
        // The editor starts in INSERT mode.
        TextEditorVi v("", "", {});
        assert(v.get_mode() == TextEditor::Mode::INSERT);
        v.edit("ab");
        assert(v.get_lhs() == "ab");
    }

    {
        // ^H (0x08) and DEL (0x7F) both delete one character backward in INSERT mode.
        TextEditorVi v("abc", "", {});
        v.edit("\x08");  // ^H: backspace
        assert(v.get_lhs() == "ab");
        v.edit("\x7F");  // DEL: backspace
        assert(v.get_lhs() == "a");
    }

    {
        // Arrow keys in INSERT mode move the cursor.
        TextEditorVi v("abc", "", {});
        v.edit(StringView("\x1B[D", 3));  // Left arrow: one char backward.
        assert(v.get_lhs() == "ab");
        assert(v.get_rhs() == "c");
        v.edit(StringView("\x1B[C", 3));  // Right arrow: one char forward.
        assert(v.get_lhs() == "abc");
    }

    ////////////////////////////////////////////////////////
    // Transition: INSERT → NORMAL via ESC
    ////////////////////////////////////////////////////////

    {
        TextEditorVi v("ls ", "", {});
        v.edit("\x1B");
        assert(v.get_mode() == TextEditor::Mode::NORMAL);
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: cursor movement (h/l, 0/$)
    ////////////////////////////////////////////////////////

    {
        TextEditorVi v("abc", "", {});
        v.edit("\x1B");   // → NORMAL
        v.edit("h");      // h: move one char left
        assert(v.get_lhs() == "ab");
        assert(v.get_rhs() == "c");
        v.edit("l");      // l: move one char right
        assert(v.get_lhs() == "abc");
        assert(v.get_rhs() == "");
    }

    {
        TextEditorVi v("abc", "", {});
        v.edit("\x1B");   // → NORMAL
        v.edit("0");      // 0: move to beginning of line
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "abc");
        v.edit("$");      // $: move to end of line
        assert(v.get_lhs() == "abc");
        assert(v.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: cursor movement (^: first non-blank)
    ////////////////////////////////////////////////////////

    {
        TextEditorVi v("  hello", "", {});
        v.edit("\x1B");   // → NORMAL
        v.edit("^");      // ^: move to first non-blank character
        assert(v.get_lhs() == "  ");
        assert(v.get_rhs() == "hello");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: deletion (x/X, D)
    ////////////////////////////////////////////////////////

    {
        // x: delete the character under the cursor (first char of rhs).
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");
        v.edit("x");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "bc");
    }

    {
        // X: backspace (delete one character before the cursor).
        TextEditorVi v("abc", "", {});
        v.edit("\x1B");
        v.edit("X");
        assert(v.get_lhs() == "ab");
    }

    {
        // D: delete from the cursor to the end of the line.
        TextEditorVi v("ab", "cd", {});
        v.edit("\x1B");
        v.edit("D");
        assert(v.get_lhs() == "ab");
        assert(v.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: substitute (S, s) and change-to-EOL (C)
    ////////////////////////////////////////////////////////

    {
        // S: clear the whole line and enter INSERT mode.
        TextEditorVi v("hello", "", {});
        v.edit("\x1B");
        v.edit("S");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
    }

    {
        // s: delete char at cursor and enter INSERT mode.
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");
        v.edit("s");
        assert(v.get_rhs() == "bc");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
    }

    {
        // C: yank then delete to end of line, then enter INSERT mode.
        TextEditorVi v("ab", "cd", {});
        v.edit("\x1B");
        v.edit("C");
        assert(v.get_lhs() == "ab");
        assert(v.get_rhs() == "");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: mode transitions (i/I/a/A)
    ////////////////////////////////////////////////////////

    {
        // i: enter INSERT mode at the current cursor position.
        TextEditorVi v("abc", "", {});
        v.edit("\x1B");
        v.edit("h");    // move to 'ab|c'
        v.edit("i");    // INSERT before 'c'
        assert(v.get_mode() == TextEditor::Mode::INSERT);
        v.edit("X");    // insert 'X'
        assert(v.get_lhs() == "abX");
        assert(v.get_rhs() == "c");
    }

    {
        // I: enter INSERT mode at the beginning of the line.
        TextEditorVi v("abc", "", {});
        v.edit("\x1B");
        v.edit("I");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "abc");
    }

    {
        // a: enter INSERT mode after the character at the cursor.
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");
        v.edit("a");    // append: cursor moves one char right
        assert(v.get_mode() == TextEditor::Mode::INSERT);
        assert(v.get_lhs() == "a");
    }

    {
        // A: enter INSERT mode at the end of the line.
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");
        v.edit("A");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
        assert(v.get_lhs() == "abc");
        assert(v.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: word motions (w/b/e, W/B)
    ////////////////////////////////////////////////////////

    {
        // w: advance cursor to the start of the next word.
        TextEditorVi v("", "hello world", {});
        v.edit("\x1B");
        v.edit("w");
        assert(v.get_lhs() == "hello ");
        assert(v.get_rhs() == "world");
    }

    {
        // b: retreat cursor to the start of the previous word.
        TextEditorVi v("hello world", "", {});
        v.edit("\x1B");
        v.edit("b");
        assert(v.get_lhs() == "hello ");
        assert(v.get_rhs() == "world");
    }

    {
        // e: advance cursor to the last character of the current/next word.
        TextEditorVi v("", "hello world", {});
        v.edit("\x1B");
        v.edit("e");    // cursor lands on 'o' (last char of "hello")
        assert(v.get_lhs() == "hell");
        assert(String(v.get_rhs()).starts_with("o"));
    }

    {
        // W: bigword forward (skip non-space sequence including punctuation).
        TextEditorVi v("", "hello.world foo", {});
        v.edit("\x1B");
        v.edit("W");
        assert(v.get_lhs() == "hello.world ");
        assert(v.get_rhs() == "foo");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: history navigation (k/j)
    ////////////////////////////////////////////////////////

    {
        Deque<String> hists = {"cmd1", "cmd2"};
        TextEditorVi v("", "", hists);
        v.edit("\x1B");   // → NORMAL
        v.edit("k");      // k: go to most-recent history entry "cmd2"
        assert(v.get_lhs() == "cmd2");
        v.edit("k");      // k: go to older entry "cmd1"
        assert(v.get_lhs() == "cmd1");
        v.edit("j");      // j: advance toward current buffer ("cmd2")
        assert(v.get_lhs() == "cmd2");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: case toggle (~)
    ////////////////////////////////////////////////////////

    {
        // ~: toggle the case of the character under the cursor.
        TextEditorVi v("", "hello", {});
        v.edit("\x1B");
        v.edit("~");   // 'h' → 'H'
        assert(v.get_lhs() == "H");
        assert(String(v.get_rhs()).starts_with("e"));
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: operator + motion (dd, dw, yy, cc)
    ////////////////////////////////////////////////////////

    {
        // dd: delete the whole line.
        TextEditorVi v("hello ", "world", {});
        v.edit("\x1B");
        v.edit("d");
        v.edit("d");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "");
    }

    {
        // dw: delete one word forward.
        TextEditorVi v("", "hello world", {});
        v.edit("\x1B");
        v.edit("d");
        v.edit("w");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "world");
    }

    {
        // yy: yank the whole line (buffer is not modified).
        TextEditorVi v("hello ", "world", {});
        v.edit("\x1B");
        v.edit("y");
        v.edit("y");
        assert(v.get_lhs() == "hello ");
        assert(v.get_rhs() == "world");
    }

    {
        // cc: change whole line (clear buffer and enter INSERT mode).
        TextEditorVi v("hello ", "world", {});
        v.edit("\x1B");
        v.edit("c");
        v.edit("c");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: replace (r) and paste (p/P)
    ////////////////////////////////////////////////////////

    {
        // r: replace the character under the cursor with the next typed character.
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");
        v.edit("r");   // arm the replace operator
        v.edit("X");   // replace 'a' with 'X'; cursor stays on 'X'
        assert(v.get_rhs() == "Xbc");
    }

    {
        // p: paste the yank buffer after the cursor.
        TextEditorVi v("", "xyz", {});
        v.edit("\x1B");
        v.edit("y"); v.edit("y");  // yy: yank "xyz"
        v.edit("d"); v.edit("d");  // dd: delete whole line (buffer now empty)
        v.edit("p");               // paste yank buffer after cursor
        assert(v.get_lhs() == "xyz");
    }

    {
        // P: paste the yank buffer before the cursor.
        TextEditorVi v("", "xyz", {});
        v.edit("\x1B");
        v.edit("y"); v.edit("y");  // yy: yank "xyz"
        v.edit("d"); v.edit("d");  // dd: delete whole line
        v.edit("P");               // paste before cursor
        assert(v.get_lhs() == "xyz");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: ESC cancels a pending operator
    ////////////////////////////////////////////////////////

    {
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");    // → NORMAL
        v.edit("d");       // pending_op = 'd'
        v.edit("\x1B");    // ESC: cancel the pending operator
        v.edit("x");       // x should delete 'a', not apply 'd'
        assert(v.get_rhs() == "bc");
    }

    ////////////////////////////////////////////////////////
    // Arrow keys in NORMAL mode
    ////////////////////////////////////////////////////////

    {
        TextEditorVi v("abc", "", {});
        v.edit("\x1B");                   // → NORMAL
        v.edit(StringView("\x1B[D", 3));  // Left arrow: move one char left.
        assert(v.get_lhs() == "ab");
        assert(v.get_rhs() == "c");
    }

    ////////////////////////////////////////////////////////
    // INSERT mode: control character encoding (ins_ctrl)
    ////////////////////////////////////////////////////////

    {
        // Ctrl-A (0x01) in INSERT mode is rendered as "^A".
        TextEditorVi v("", "", {});
        v.edit("\x01");
        assert(v.get_lhs() == "^A");
    }

    {
        // Ctrl-C (0x03) in INSERT mode is rendered as "^C".
        TextEditorVi v("", "", {});
        v.edit("\x03");
        assert(v.get_lhs() == "^C");
    }

    {
        // Ctrl-Z (0x1A) in INSERT mode is rendered as "^Z".
        TextEditorVi v("", "", {});
        v.edit("\x1A");
        assert(v.get_lhs() == "^Z");
    }

    ////////////////////////////////////////////////////////
    // INSERT mode: Up/Down arrow keys navigate history
    ////////////////////////////////////////////////////////

    {
        Deque<String> hists = {"cmd1", "cmd2"};
        TextEditorVi v("", "", hists);

        // Up arrow in INSERT mode navigates to most-recent history entry "cmd2".
        v.edit(StringView("\x1B[A", 3));
        assert(v.get_lhs() == "cmd2");

        // Down arrow in INSERT mode returns to the editing buffer.
        v.edit(StringView("\x1B[B", 3));
        assert(v.get_lhs() == "");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: B (bigword backward) and E (bigword word-end)
    ////////////////////////////////////////////////////////

    {
        // B: retreat to the start of the previous BIGWORD (treats "hello.world" as one token).
        TextEditorVi v("hello.world", "", {});
        v.edit("\x1B");
        v.edit("B");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "hello.world");
    }

    {
        // B: skip trailing space then the whole bigword.
        TextEditorVi v("foo bar ", "", {});
        v.edit("\x1B");
        v.edit("B");
        assert(v.get_lhs() == "foo ");
        assert(v.get_rhs() == "bar ");
    }

    {
        // E: advance to the last character of the current BIGWORD (ignores punctuation boundaries).
        // "foo.bar baz": E (bigword) stops at 'r', the last char of "foo.bar".
        TextEditorVi v("", "foo.bar baz", {});
        v.edit("\x1B");
        v.edit("E");
        assert(v.get_lhs() == "foo.ba");
        assert(String(v.get_rhs()).starts_with("r"));
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: right/down/up arrow keys
    ////////////////////////////////////////////////////////

    {
        // Right arrow in NORMAL mode moves cursor one char right.
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");
        v.edit(StringView("\x1B[C", 3));
        assert(v.get_lhs() == "a");
        assert(v.get_rhs() == "bc");
    }

    {
        // Down/Up arrow in NORMAL mode navigate history.
        Deque<String> hists = {"cmd1", "cmd2"};
        TextEditorVi v("", "", hists);
        v.edit("\x1B");
        v.edit(StringView("\x1B[A", 3));  // Up arrow → "cmd2"
        assert(v.get_lhs() == "cmd2");
        v.edit(StringView("\x1B[B", 3));  // Down arrow → back to editing buffer
        assert(v.get_lhs() == "");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: input with unsupported size is silently ignored
    ////////////////////////////////////////////////////////

    {
        // A 2-byte input (size != 1 and size != 3) must be silently discarded.
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");
        v.edit(StringView("xy", 2));
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "abc");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: ~ on uppercase, non-alpha, and empty rhs
    ////////////////////////////////////////////////////////

    {
        // ~ on an uppercase letter toggles it to lowercase and advances the cursor.
        TextEditorVi v("", "Hello", {});
        v.edit("\x1B");
        v.edit("~");
        assert(v.get_lhs() == "h");   // 'H' → 'h', cursor moved past it
        assert(String(v.get_rhs()).starts_with("e"));
    }

    {
        // ~ on a non-alpha character (digit) just advances the cursor.
        TextEditorVi v("", "1abc", {});
        v.edit("\x1B");
        v.edit("~");
        assert(v.get_lhs() == "1");
        assert(v.get_rhs() == "abc");
    }

    {
        // ~ on empty rhs is a no-op.
        TextEditorVi v("abc", "", {});
        v.edit("\x1B");
        v.edit("~");
        assert(v.get_lhs() == "abc");
        assert(v.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: operator + $ and 0 motions
    ////////////////////////////////////////////////////////

    {
        // d$: delete from cursor to end of line.
        TextEditorVi v("ab", "cd", {});
        v.edit("\x1B");
        v.edit("d"); v.edit("$");
        assert(v.get_lhs() == "ab");
        assert(v.get_rhs() == "");
    }

    {
        // d0: delete from beginning of line to cursor.
        TextEditorVi v("ab", "cd", {});
        v.edit("\x1B");
        v.edit("d"); v.edit("0");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "cd");
    }

    {
        // c$: change to end of line — delete rhs and enter INSERT mode.
        TextEditorVi v("ab", "cd", {});
        v.edit("\x1B");
        v.edit("c"); v.edit("$");
        assert(v.get_lhs() == "ab");
        assert(v.get_rhs() == "");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
    }

    {
        // c0: change to beginning of line — delete lhs and enter INSERT mode.
        TextEditorVi v("ab", "cd", {});
        v.edit("\x1B");
        v.edit("c"); v.edit("0");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "cd");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
    }

    {
        // y$: yank to end of line without modifying the buffer.
        // Verify by pasting (P inserts at current position).
        TextEditorVi v("ab", "cd", {});
        v.edit("\x1B");
        v.edit("y"); v.edit("$");
        assert(v.get_lhs() == "ab");   // buffer unchanged
        assert(v.get_rhs() == "cd");
        v.edit("P");                   // paste yanked "cd" before cursor
        assert(v.get_lhs() == "abcd");
        assert(v.get_rhs() == "cd");
    }

    {
        // y0: yank to beginning of line without modifying the buffer.
        TextEditorVi v("abc", "def", {});
        v.edit("\x1B");
        v.edit("y"); v.edit("0");
        assert(v.get_lhs() == "abc");  // buffer unchanged
        assert(v.get_rhs() == "def");
        v.edit("P");                   // paste yanked "abc" before cursor
        assert(v.get_lhs() == "abcabc");
        assert(v.get_rhs() == "def");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: operator + ^ motion
    ////////////////////////////////////////////////////////

    {
        // d^ (cursor is after non-blank region): delete backward to first non-blank.
        // lhs="  hello world", rhs="": first non-blank is at position 2,
        // cursor is at 13, so delete the last 11 chars of lhs.
        TextEditorVi v("  hello world", "", {});
        v.edit("\x1B");
        v.edit("d"); v.edit("^");
        assert(v.get_lhs() == "  ");
        assert(v.get_rhs() == "");
    }

    {
        // d^ (cursor is before first non-blank): delete forward to first non-blank.
        // lhs="", rhs="  hello": first non-blank is at position 2,
        // cursor is at 0, so delete first 2 chars of rhs.
        TextEditorVi v("", "  hello", {});
        v.edit("\x1B");
        v.edit("d"); v.edit("^");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "hello");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: operator + word-backward motions (db, dB, cb, cW)
    ////////////////////////////////////////////////////////

    {
        // db: delete one word backward.
        TextEditorVi v("hello world", "", {});
        v.edit("\x1B");
        v.edit("d"); v.edit("b");
        assert(v.get_lhs() == "hello ");
        assert(v.get_rhs() == "");
    }

    {
        // dB: delete one bigword backward (treats "hello.world" as one token).
        TextEditorVi v("hello.world ", "", {});
        v.edit("\x1B");
        v.edit("d"); v.edit("B");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "");
    }

    {
        // cb: change word backward — delete word and enter INSERT mode.
        TextEditorVi v("hello world", "", {});
        v.edit("\x1B");
        v.edit("c"); v.edit("b");
        assert(v.get_lhs() == "hello ");
        assert(v.get_rhs() == "");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
    }

    {
        // cW: change bigword forward — delete bigword and enter INSERT mode.
        TextEditorVi v("", "hello.world foo", {});
        v.edit("\x1B");
        v.edit("c"); v.edit("W");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "foo");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: operator + word-end motions (de, dE)
    ////////////////////////////////////////////////////////

    {
        // de: delete up to and including the last char of the current word.
        // "hello world": 'e' lands on position 4 ('o'), so de deletes "hello" (5 chars).
        TextEditorVi v("", "hello world", {});
        v.edit("\x1B");
        v.edit("d"); v.edit("e");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == " world");
    }

    {
        // dE: delete up to and including the last char of the current BIGWORD.
        // "foo.bar baz": E (bigword) lands on 'r', so dE deletes "foo.bar" (7 chars).
        TextEditorVi v("", "foo.bar baz", {});
        v.edit("\x1B");
        v.edit("d"); v.edit("E");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == " baz");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: operator + W motion (dW, cw)
    ////////////////////////////////////////////////////////

    {
        // dW: delete one bigword forward including trailing space.
        TextEditorVi v("", "hello.world foo", {});
        v.edit("\x1B");
        v.edit("d"); v.edit("W");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "foo");
    }

    {
        // cw: change word forward — delete word and enter INSERT mode.
        TextEditorVi v("", "hello world", {});
        v.edit("\x1B");
        v.edit("c"); v.edit("w");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "world");
        assert(v.get_mode() == TextEditor::Mode::INSERT);
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: yank + word motions (yw, yb)
    ////////////////////////////////////////////////////////

    {
        // yw: yank word forward without modifying the buffer.
        // Verify yank content by pasting with P.
        TextEditorVi v("", "hello world", {});
        v.edit("\x1B");
        v.edit("y"); v.edit("w");
        assert(v.get_lhs() == "");          // buffer must be unchanged
        assert(v.get_rhs() == "hello world");
        v.edit("P");                         // paste yanked "hello " before cursor
        assert(v.get_lhs() == "hello ");
        assert(v.get_rhs() == "hello world");
    }

    {
        // yb: yank word backward without modifying the buffer.
        // Verify yank content by pasting with P.
        TextEditorVi v("hello world", "", {});
        v.edit("\x1B");
        v.edit("y"); v.edit("b");
        assert(v.get_lhs() == "hello world"); // buffer must be unchanged
        assert(v.get_rhs() == "");
        v.edit("P");                           // paste yanked "world" at current position
        assert(v.get_lhs() == "hello worldworld");
        assert(v.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: r on empty rhs (no-op)
    ////////////////////////////////////////////////////////

    {
        // r followed by a character when rhs is empty: no replacement takes place.
        TextEditorVi v("abc", "", {});
        v.edit("\x1B");
        v.edit("r"); v.edit("X");
        assert(v.get_lhs() == "abc");
        assert(v.get_rhs() == "");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: unknown motion after operator (silently cancelled)
    ////////////////////////////////////////////////////////

    {
        // 'z' is not a valid motion; the pending 'd' is discarded and the buffer is untouched.
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");
        v.edit("d"); v.edit("z");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "abc");
    }

    ////////////////////////////////////////////////////////
    // NORMAL mode: p/P with empty yank buffer
    ////////////////////////////////////////////////////////

    {
        // p with empty yank buffer: cursor advances one right (rhs non-empty) but nothing is inserted.
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");
        v.edit("p");
        assert(v.get_lhs() == "a");
        assert(v.get_rhs() == "bc");
    }

    {
        // P with empty yank buffer: cursor does not move and nothing is inserted.
        TextEditorVi v("", "abc", {});
        v.edit("\x1B");
        v.edit("P");
        assert(v.get_lhs() == "");
        assert(v.get_rhs() == "abc");
    }

}   // }}}

static void test_tokenizers(void)
{   // {{{

    // Print header.
    print_header("Unit test for tokenizers.cxx");

    ////////////////////////////////////////////////////////
    // TOKENIZE_PLAIN: default mode, no whitespace tokens
    ////////////////////////////////////////////////////////

    {
        uint32_t count = 0;
        for (const StringView sv : tokenize("ls -la /tmp", TOKENIZE_PLAIN))
        {
            switch (count++)
            {
                case 0: assert(sv == "ls");   break;
                case 1: assert(sv == "-la");  break;
                case 2: assert(sv == "/tmp"); break;
                default: assert(false);
            }
        }
        assert(count == 3);
    }

    ////////////////////////////////////////////////////////
    // TOKENIZE_KEEP_WS: whitespace tokens are preserved
    ////////////////////////////////////////////////////////

    {
        uint32_t count = 0;
        for (const StringView sv : tokenize("ls -la", TOKENIZE_KEEP_WS))
        {
            switch (count++)
            {
                case 0: assert(sv == "ls");  break;
                case 1: assert(sv == " ");   break;
                case 2: assert(sv == "-la"); break;
                default: assert(false);
            }
        }
        assert(count == 3);
    }

    ////////////////////////////////////////////////////////
    // TOKENIZE_DEQUOTE: strip surrounding single/double quotes
    ////////////////////////////////////////////////////////

    {
        // Single-quoted token should be stripped.
        uint32_t count = 0;
        for (const StringView sv : tokenize("echo 'hello world'", TOKENIZE_DEQUOTE))
        {
            switch (count++)
            {
                case 0: assert(sv == "echo");        break;
                case 1: assert(sv == "hello world"); break;
                default: assert(false);
            }
        }
        assert(count == 2);
    }

    {
        // Double-quoted token should be stripped.
        uint32_t count = 0;
        for (const StringView sv : tokenize("echo \"hello world\"", TOKENIZE_DEQUOTE))
        {
            switch (count++)
            {
                case 0: assert(sv == "echo");        break;
                case 1: assert(sv == "hello world"); break;
                default: assert(false);
            }
        }
        assert(count == 2);
    }

    ////////////////////////////////////////////////////////
    // Empty input
    ////////////////////////////////////////////////////////

    {
        uint32_t count = 0;
        for ([[maybe_unused]] const StringView sv : tokenize("", TOKENIZE_PLAIN))
            ++count;
        assert(count == 0);
    }

    ////////////////////////////////////////////////////////
    // tokenize_with_placeholder_replacement: tilde expansion
    ////////////////////////////////////////////////////////

    {
        // Tilde should be expanded to the home directory.
        StringMap extra;
        uint32_t count = 0;
        for (const String& s : tokenize_with_placeholder_replacement("ls ~/docs", extra, TOKENIZE_PLAIN))
        {
            if (count == 1)
                assert(s.find("docs") != String::npos and not s.starts_with("~"));
            ++count;
        }
        assert(count == 2);
    }

    ////////////////////////////////////////////////////////
    // tokenize_with_placeholder_replacement: extra map replacement
    ////////////////////////////////////////////////////////

    {
        // A token matching a key in the extra map should be replaced by the value.
        StringMap extra;
        extra["{key}"] = "value";

        uint32_t count = 0;
        for (const String& s : tokenize_with_placeholder_replacement("echo {key}", extra, TOKENIZE_PLAIN))
        {
            if (count == 1)
                assert(s == "value");
            ++count;
        }
        assert(count == 2);
    }

}   // }}}

static void test_utf8(void)
{   // {{{

    // Print header.
    print_header("Unit test for utf8.cxx");

    ////////////////////////////////////////////////////////
    // utf8_byte_size: infer byte count from leading byte
    ////////////////////////////////////////////////////////

    // ASCII (1-byte) characters have a leading byte in 0x00–0x7F range.
    assert(utf8_byte_size(0x41) == 1);  // 'A'
    assert(utf8_byte_size(0x7F) == 1);  // DEL

    // 2-byte UTF-8 leading byte: 0xC0–0xDF.
    assert(utf8_byte_size(0xC3) == 2);  // e.g. Latin Extended

    // 3-byte UTF-8 leading byte: 0xE0–0xEF.
    assert(utf8_byte_size(0xE3) == 3);  // e.g. CJK characters (あ = 0xE3 0x81 0x82)

    // 4-byte UTF-8 leading byte: 0xF0–0xF7.
    assert(utf8_byte_size(0xF0) == 4);  // e.g. supplementary characters

    ////////////////////////////////////////////////////////
    // utf8_width: display width per codepoint
    ////////////////////////////////////////////////////////

    assert(utf8_width(0x0041) == 1);  // 'A' (ASCII)
    assert(utf8_width(0x3042) == 2);  // 'あ' (hiragana)
    assert(utf8_width(0x4E2D) == 2);  // '中' (CJK unified ideograph)

    ////////////////////////////////////////////////////////
    // utf8_decode_iter: iterate codepoints of a UTF-8 string
    ////////////////////////////////////////////////////////

    String str = "Aあ𩸽";
    Vector<uint32_t> codepoints;
    for (const auto& [codepoint, ptr] : utf8_decode_iter(str.data(), str.size()))
        codepoints.push_back(codepoint);
    assert(codepoints.size() == 3);
    assert(codepoints[0] == 0x0041);    // 'A'
    assert(codepoints[1] == 0x3042);    // 'あ'
    assert(codepoints[2] == 0x029e3d);  // '𩸽'

    ////////////////////////////////////////////////////////
    // utf8_encode: encode a Unicode codepoint to UTF-8
    ////////////////////////////////////////////////////////

    uint8_t buffer[5] = {0};

    // 1-byte encoding (ASCII).
    utf8_encode(0x0041, buffer);
    assert(std::strcmp(reinterpret_cast<const char*>(buffer), "A") == 0);

    // 3-byte encoding (hiragana).
    utf8_encode(0x3042, buffer);
    assert(std::strcmp(reinterpret_cast<const char*>(buffer), "あ") == 0);

    // 4-byte encoding (supplementary character).
    utf8_encode(0x029e3d, buffer);
    assert(std::strcmp(reinterpret_cast<const char*>(buffer), "𩸽") == 0);

    ////////////////////////////////////////////////////////
    // utf8_decode: single character decode
    ////////////////////////////////////////////////////////

    {
        const uint8_t* p = reinterpret_cast<const uint8_t*>("あ");
        int32_t cp = 0;
        ptrdiff_t nbytes = utf8_decode(p, 3, &cp);
        assert(nbytes == 3);
        assert(cp == 0x3042);
    }

    // ASCII decode.
    {
        const uint8_t* p = reinterpret_cast<const uint8_t*>("A");
        int32_t cp = 0;
        ptrdiff_t nbytes = utf8_decode(p, 1, &cp);
        assert(nbytes == 1);
        assert(cp == 0x0041);
    }

    // 2-byte decode: U+00E9 LATIN SMALL LETTER E WITH ACUTE (é = 0xC3 0xA9).
    {
        const uint8_t p[] = {0xC3, 0xA9, 0x00};
        int32_t cp = 0;
        ptrdiff_t nbytes = utf8_decode(p, 2, &cp);
        assert(nbytes == 2);
        assert(cp == 0x00E9);
    }

    // 3-byte overlong encoding: 0xE0 0x80 0x80 decodes to U+0000 which is < 0x800,
    // so utf8_decode must reject it as invalid.
    {
        const uint8_t overlong3[] = {0xE0, 0x80, 0x80};
        int32_t cp = 0;
        assert(utf8_decode(overlong3, 3, &cp) < 0);
    }

    ////////////////////////////////////////////////////////
    // utf8_iter: iterate raw UTF-8 character string views
    ////////////////////////////////////////////////////////

    {
        String s = "Aあ";
        Vector<StringView> views;
        for (const StringView sv : utf8_iter(s.data(), s.size()))
            views.push_back(sv);
        assert(views.size() == 2);
        assert(views[0] == "A");
        assert(views[1] == "あ");
    }

    ////////////////////////////////////////////////////////
    // utf8_decode_next_charx: decode next character into CharX
    ////////////////////////////////////////////////////////

    {
        const char* p = "あい";
        CharX cx = utf8_decode_next_charx(p);
        assert(cx.size() == 3);       // "あ" is 3 bytes.
        assert(cx.view() == "あ");
    }

    {
        const char* p = "Abc";
        CharX cx = utf8_decode_next_charx(p);
        assert(cx.size() == 1);
        assert(cx.view() == "A");
    }

    ////////////////////////////////////////////////////////
    // utf8_byte_size: invalid leading bytes return 0
    ////////////////////////////////////////////////////////

    // Continuation bytes (0x80–0xBF) are not valid leading bytes.
    assert(utf8_byte_size(0x80) == 0);
    assert(utf8_byte_size(0xBF) == 0);

    // 0xFF is also an invalid leading byte.
    assert(utf8_byte_size(0xFF) == 0);

    ////////////////////////////////////////////////////////
    // utf8_decode: edge cases and error paths
    ////////////////////////////////////////////////////////

    {
        int32_t cp = 0;

        // Null pointer returns 0 without crashing.
        assert(utf8_decode(nullptr, 3, &cp) == 0);

        // Size 0 returns 0 immediately.
        const uint8_t* p = reinterpret_cast<const uint8_t*>("A");
        assert(utf8_decode(p, 0, &cp) == 0);

        // Null-terminator as first byte returns 0.
        const uint8_t nul = '\0';
        assert(utf8_decode(&nul, 1, &cp) == 0);

        // Invalid leading byte (continuation byte 0x81) returns error.
        const uint8_t bad1[] = {0x81, 0x80};
        assert(utf8_decode(bad1, 2, &cp) < 0);

        // 2-byte sequence with invalid continuation (0x40 is not 0x80–0xBF).
        const uint8_t bad2[] = {0xC3, 0x40};
        assert(utf8_decode(bad2, 2, &cp) < 0);

        // 3-byte sequence with invalid continuation byte in second position.
        const uint8_t bad3[] = {0xE3, 0x40, 0x82};
        assert(utf8_decode(bad3, 3, &cp) < 0);

        // 3-byte surrogate half (U+D800 = 0xED 0xA0 0x80) is invalid.
        const uint8_t surr[] = {0xED, 0xA0, 0x80};
        assert(utf8_decode(surr, 3, &cp) < 0);

        // 4-byte: 0xF0 requires the second byte >= 0x90.
        const uint8_t f0low[] = {0xF0, 0x80, 0x80, 0x80};
        assert(utf8_decode(f0low, 4, &cp) < 0);

        // 4-byte: 0xF4 requires the second byte <= 0x8F.
        const uint8_t f4hi[] = {0xF4, 0x90, 0x80, 0x80};
        assert(utf8_decode(f4hi, 4, &cp) < 0);

        // 4-byte: invalid continuation byte in the third position.
        const uint8_t bad4[] = {0xF0, 0x90, 0x40, 0x80};
        assert(utf8_decode(bad4, 4, &cp) < 0);
    }

    ////////////////////////////////////////////////////////
    // utf8_encode: edge cases
    ////////////////////////////////////////////////////////

    {
        uint8_t buf[5] = {0};

        // Negative codepoint is invalid — returns 0.
        assert(utf8_encode(-1, buf) == 0);

        // 2-byte encoding (U+0080).
        assert(utf8_encode(0x0080, buf) == 2);
        assert((buf[0] & 0xE0) == 0xC0);  // Leading byte: 110xxxxx.
        assert((buf[1] & 0xC0) == 0x80);  // Continuation byte: 10xxxxxx.

        // Codepoint >= 0x110000 is out of Unicode range — returns 0.
        assert(utf8_encode(0x110000, buf) == 0);
    }

    ////////////////////////////////////////////////////////
    // utf8_decode_next_charx: null pointer returns empty CharX
    ////////////////////////////////////////////////////////

    {
        const CharX cx = utf8_decode_next_charx(nullptr);
        assert(cx.size() == 0);
    }

    ////////////////////////////////////////////////////////
    // utf8_width: out-of-range codepoints use the fallback property
    ////////////////////////////////////////////////////////

    // These must not crash; the return value for invalid codepoints is implementation-defined
    // but should be non-negative.
    assert(utf8_width(-1) >= 0);
    assert(utf8_width(0x110000) >= 0);

}   // }}}

static void test_utils(void)
{   // {{{

    // Print header.
    print_header("Unit test for utils.cxx");

    ////////////////////////////////////////////////////////
    // min / max / clip templates
    ////////////////////////////////////////////////////////

    assert(min(3, 5)  == 3);
    assert(min(-1, 1) == -1);
    assert(max(3, 5)  == 5);
    assert(max(-1, 1) == 1);
    assert(clip(5, 0, 10)  == 5);   // Within range.
    assert(clip(-1, 0, 10) == 0);   // Below lower bound.
    assert(clip(15, 0, 10) == 10);  // Above upper bound.

    ////////////////////////////////////////////////////////
    // deduplicate: in-place unique + sort
    ////////////////////////////////////////////////////////

    {
        Vector<int> v = {3, 1, 2, 1, 3};
        deduplicate(v);
        assert(v.size() == 3);
        assert(v[0] == 1);
        assert(v[1] == 2);
        assert(v[2] == 3);
    }

    // Empty vector stays empty.
    {
        Vector<int> v;
        deduplicate(v);
        assert(v.empty());
    }

    ////////////////////////////////////////////////////////
    // split: generator-based string splitting
    ////////////////////////////////////////////////////////

    uint32_t count = 0;
    for (const StringView sv : split("this,is,csv", ","))
    {
        switch (count++)
        {
            case 0 : assert(sv == "this"); break;
            case 1 : assert(sv == "is");   break;
            case 2 : assert(sv == "csv");  break;
            default: assert(false);
        }
    }

    // Single token (no delimiter found): yields the whole string.
    {
        uint32_t n = 0;
        for (const StringView sv : split("nodel", ","))
        {
            assert(sv == "nodel");
            ++n;
        }
        assert(n == 1);
    }

    // Empty delimiter yields the whole string.
    {
        uint32_t n = 0;
        for (const StringView sv : split("abc", ""))
        {
            assert(sv == "abc");
            ++n;
        }
        assert(n == 1);
    }

    ////////////////////////////////////////////////////////
    // replace: substring replacement
    ////////////////////////////////////////////////////////

    assert(replace("hello world", "world", "Japan") == "hello Japan");
    assert(replace("aaa", "a", "bb") == "bbbbbb");

    // No match: original string returned.
    assert(replace("hello", "xyz", "abc") == "hello");

    // Replace with empty string (effectively deletes the old string).
    assert(replace("hello world", "world", "") == "hello ");

    ////////////////////////////////////////////////////////
    // strip: whitespace trimming
    ////////////////////////////////////////////////////////

    assert(strip("  hello  ") == "hello");
    assert(strip("  hello  ", true, false) == "hello  ");   // left only.
    assert(strip("  hello  ", false, true) == "  hello");   // right only.
    assert(strip("hello") == "hello");                       // No whitespace.
    assert(strip("   ") == "");                              // All whitespace.

    ////////////////////////////////////////////////////////
    // expand_tilde: tilde expansion
    ////////////////////////////////////////////////////////

    assert(expand_tilde("~/.config").ends_with("/.config"));
    assert(expand_tilde(".config").ends_with(".config"));

    // Path without tilde prefix is returned unchanged.
    assert(expand_tilde("/usr/local") == "/usr/local");

    ////////////////////////////////////////////////////////
    // hash: FNV-1a hash function
    ////////////////////////////////////////////////////////

    // The same string should always produce the same hash.
    assert(hash("hello") == hash("hello"));

    // Different strings should (almost always) produce different hashes.
    assert(hash("hello") != hash("world"));

    // Null string should return the initial hash value.
    const char* null_str = nullptr;
    assert(hash(null_str) == 0xcbf29ce484222325ULL);

    ////////////////////////////////////////////////////////
    // get_time: formatted time string
    ////////////////////////////////////////////////////////

    time_t raw_time = std::time(nullptr);
    assert(get_time(raw_time, "%Y/%m/%d").size() > 0);

    ////////////////////////////////////////////////////////
    // readline: generator-based file reading
    ////////////////////////////////////////////////////////

    // Non-existent file yields no lines.
    {
        uint32_t n = 0;
        for ([[maybe_unused]] const String& line : readline("/non_existent_file_xyz"))
            ++n;
        assert(n == 0);
    }

    // Existing file (this test file itself) should have multiple lines.
    {
        uint32_t n = 0;
        for ([[maybe_unused]] const String& line : readline("test_main.cxx"))
            ++n;
        assert(n > 10);
    }

    ////////////////////////////////////////////////////////
    // logo: print the ExBash logo (visual check)
    ////////////////////////////////////////////////////////

    print_exbash_logo();

    ////////////////////////////////////////////////////////
    // tokenize with TOKENIZE_DEQUOTE option
    ////////////////////////////////////////////////////////

    count = 0;
    for (const StringView sv : tokenize("timeout 0.1s 'ls --help'", TOKENIZE_DEQUOTE))
    {
        switch (count++)
        {
            case 0 : assert(sv == "timeout");    break;
            case 1 : assert(sv == "0.1s");       break;
            case 2 : assert(sv == "ls --help");  break;
            default: assert(false);
        }
    }

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Integration test functions
////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_readcmd(void)
{   // {{{

    // Skip this test when stdin is not a terminal, because readcmd requires a TTY.
    if (not isatty(STDIN_FILENO) or not isatty(STDOUT_FILENO))
    {
        print_header("Integration test for readcmd function (SKIPPED: no TTY)");
        return;
    }

    // Load the test config used in all readcmd tests.
    const ExBashConfig cfg = load_config("misc/config.toml");

    const auto run_test_readcmd = [&cfg](const char* input, const char* output_lhs, const char* output_rhs) -> bool
    // Run readcmd for testing purpose.
    //
    // [Args]
    //   input      (const char*): Input command string.
    //   output_lhs (const char*): Expected lhs string after the command.
    //   output_rhs (const char*): Expected rhs string after the command.
    //
    // [Returns]
    //   (bool): True if the output matches the expected values.
    {
        const char* lhs_ini = "";
        const char* rhs_ini = "";
        const Deque<String> hists = {"previous input1", "previous input2"};

        try
        {
            ReadCmdOut rc_out = readcmd(lhs_ini, rhs_ini, hists, "emacs", input, cfg);

            if (rc_out.stop.size() > 0)
                rc_out.lhs = rc_out.stop;

            return (String(output_lhs) == rc_out.lhs) and (String(output_rhs) == rc_out.rhs);
        }
        catch (const std::exception& e)
        {
            std::cerr << "readcmd exception: " << e.what() << std::endl;
            return false;
        }
    };

    // Print header.
    print_header("Unit test for readcmd function");

    // Command completions.
    assert(run_test_readcmd("ls -l\n", "ls -l", ""));
    assert(run_test_readcmd("ls ~/\n", "ls ~/", ""));
    assert(run_test_readcmd("ls Makefile \n", "ls Makefile ", ""));
    assert(run_test_readcmd("git bra\t\n", "git branch ", ""));
    assert(run_test_readcmd("ls ../tests\t \n", "ls ../tests/ ", ""));
    assert(run_test_readcmd("ls ./source/cxx/conf\t \n", "ls ./source/cxx/config. ", ""));

    // Simple typing followed by Enter.
    assert(run_test_readcmd("p\n", "p", ""));

    // History completions.
    assert(run_test_readcmd("previ\x05\n", "previous input2 ", ""));
    assert(run_test_readcmd("previous input1\x05\n", "previous input1 ", ""));

    // Test the stop key.
    assert(run_test_readcmd("\x06\n", "^F", ""));

    // Ctrl-C and Ctrl-D.
    assert(run_test_readcmd("\x03", "^C", ""));
    assert(run_test_readcmd("\x04\n", "^D", ""));

    // Test carapace.
    assert(run_test_readcmd("apk install git\n", "apk install git", ""));

    ////////////////////////////////////////////////////////
    // readcmd with non-empty rhs_ini: exercises update() rhs branch
    ////////////////////////////////////////////////////////

    {
        // When the initial rhs is non-empty, TermUserIF::update() takes the
        // "if (not rhs.empty())" branch (lines 230-232 in terminal.cxx).
        const Deque<String> hists;
        try
        {
            ReadCmdOut rc_out = readcmd("hello ", "world", hists, "emacs", "\n", cfg);
            assert(rc_out.lhs == "hello ");
            assert(rc_out.rhs == "world");
        }
        catch (const std::exception& e)
        {
            std::cerr << "readcmd (non-empty rhs) exception: " << e.what() << std::endl;
        }
    }

    ////////////////////////////////////////////////////////
    // TermUserIF::update_lines with wrong number of lines
    ////////////////////////////////////////////////////////

    {
        // update_lines returns false when the number of lines differs from the
        // number of rows the terminal was created with.
        const Size term_size = get_terminal_size();
        TermUserIF termui(cfg.area_height, term_size.cols);

        // A vector with a different number of lines than area_height triggers the early return.
        const Vector<String> wrong_lines = {"only one line"};
        assert(termui.update_lines(wrong_lines) == false);

        // A vector with exactly area_height lines should succeed.
        const Vector<String> correct_lines(cfg.area_height, "\x1B[0K");
        assert(termui.update_lines(correct_lines) == true);
    }

}   // }}}

static void test_main_exbash(void)
{   // {{{

    // Skip this test when stdin/stdout is not a terminal, because main_exbash requires a TTY.
    if (not isatty(STDIN_FILENO) or not isatty(STDOUT_FILENO))
    {
        print_header("Integration test for main_exbash (SKIPPED: no TTY)");
        return;
    }

    std::remove("/tmp/exbash_test.pipe");

    const char* argv0[] = {"exbash_readcmd", "--output", "/tmp/exbash_test.pipe"};
    main_exbash(3, const_cast<char**>(argv0), "\x14""exit\n");
    std::ofstream("/tmp/exbash_test.pipe");
    main_exbash(3, const_cast<char**>(argv0), "\x14""exit\n");

    std::remove("/tmp/exbash_test.pipe");

    const char* argv1[] = {"exbash_readcmd", "--config", "misc/config.toml"};
    main_exbash(3, const_cast<char**>(argv1), "exit\n");

    const char* argv2[] = {"exbash_readcmd", "--run", "logo"};
    main_exbash(3, const_cast<char**>(argv2), "exit\n");

    const char* argv3[] = {"exbash_readcmd", "--run", "print_path_cmds"};
    main_exbash(3, const_cast<char**>(argv3), "exit\n");

    const char* argv4[] = {"exbash_readcmd", "--run", "print_config=path_cmnd_info"};
    main_exbash(3, const_cast<char**>(argv4), "exit\n");

    const char* argv5[] = {"exbash_readcmd", "--run", "unknown"};
    main_exbash(3, const_cast<char**>(argv5), "exit\n");

    const char* argv6[] = {"exbash_readcmd", "--config", "misc/config.toml"};
    main_exbash(3, const_cast<char**>(argv6), "\x05\n");

    // Test the run_keybind path: ^F is a stop key bound to an external command.
    // The keybind command (filechooser) likely does not exist in the test environment,
    // so run_keybind will fail to open the plugin output file and fall back gracefully.
    // The subsequent Enter key causes main_exbash to exit normally.
    const char* argv7[] = {"exbash_readcmd", "--config", "misc/config.toml"};
    main_exbash(3, const_cast<char**>(argv7), "\x06\n");

    // Same test but with a mock plugin output file present, so run_keybind can read it.
    {
        std::ofstream ofs("/dev/shm/exbash/plugin.out");
        ofs << "left_part\nright_part\n";
    }
    const char* argv8[] = {"exbash_readcmd", "--config", "misc/config.toml"};
    main_exbash(3, const_cast<char**>(argv8), "\x06\n");
    std::remove("/dev/shm/exbash/plugin.out");

    // Test the --help option.
    const char* argv9[] = {"exbash_readcmd", "--help"};
    main_exbash(2, const_cast<char**>(argv9), "\x06\n");

}   // }}}

static void test_main_exbash_non_tty(void)
{   // {{{

    // Print header.
    print_header("Unit test for main_exbash (non-TTY paths)");

    ////////////////////////////////////////////////////////
    // --run generate_path_commands_cache: does not require a TTY
    ////////////////////////////////////////////////////////

    {
        const char* argv[] = {"exbash_readcmd", "--run", "generate_path_commands_cache"};
        [[maybe_unused]] int32_t ret = main_exbash(3, const_cast<char**>(argv), "");
    }

    ////////////////////////////////////////////////////////
    // --run logo: print logo and exit (does not require TTY)
    ////////////////////////////////////////////////////////

    {
        const char* argv[] = {"exbash_readcmd", "--run", "logo"};
        [[maybe_unused]] int32_t ret = main_exbash(3, const_cast<char**>(argv), "");
    }

    ////////////////////////////////////////////////////////
    // --run print_config=path_bash_info: print config value, no TTY needed
    ////////////////////////////////////////////////////////

    {
        const char* argv[] = {"exbash_readcmd", "--run", "print_config=path_bash_info"};
        [[maybe_unused]] int32_t ret = main_exbash(3, const_cast<char**>(argv), "");
    }

    ////////////////////////////////////////////////////////
    // --run unknown_xyz: unknown command → error path
    ////////////////////////////////////////////////////////

    {
        const char* argv[] = {"exbash_readcmd", "--run", "unknown_xyz_abc"};
        [[maybe_unused]] int32_t ret = main_exbash(3, const_cast<char**>(argv), "");
    }

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main function
////////////////////////////////////////////////////////////////////////////////////////////////////

static void test_EditHelper(void)
{   // {{{

    // Print header.
    print_header("Unit test for EditHelper class");

    // Load the test configuration (includes GREP entry for cat command).
    const ExBashConfig cfg = load_config("misc/config.toml");

    ////////////////////////////////////////////////////////
    // Constructor: basic construction
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        assert(true);
    }

    ////////////////////////////////////////////////////////
    // candidate("") → NONE → cands_filepath with empty tokens
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines = eh.candidate("");
        assert(lines.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // candidate("./") → PATH (starts with ./~) → cands_filepath
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines = eh.candidate("./");
        // Must return area_height lines; some files/dirs should be found.
        assert(lines.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // candidate("ls") → COMMAND (.+) → cands_command
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines = eh.candidate("ls");
        assert(lines.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // candidate("ls ") → PATH (>> .*) → cands_filepath with trailing space
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines = eh.candidate("ls ");
        assert(lines.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // candidate("ls --") → OPTION (>> -.*) → cands_option
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines = eh.candidate("ls --");
        assert(lines.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // candidate("cat Makefile ") → PREVIEW (>> FILE "") → cands_filepath + cands_preview
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        // Makefile exists in the tests/ working directory.
        const Vector<String> lines = eh.candidate("cat Makefile ");
        assert(lines.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // candidate("git ") → SUBCMD → cands_subcmd
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines = eh.candidate("git ");
        assert(lines.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // candidate("make ") → SHELL (make .*) → cands_shell
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines = eh.candidate("make ");
        assert(lines.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // candidate("cat area") → GREP (cat .*) → cands_grep
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines = eh.candidate("cat area");
        assert(lines.size() == (size_t) cfg.area_height);

        // "area_height" from misc/config.toml should be a candidate.
        bool found_area_height = false;
        for (const String& line : lines)
            if (line.find("area_height") != String::npos) { found_area_height = true; break; }
        assert(found_area_height);
    }

    ////////////////////////////////////////////////////////
    // Cache hit: cache_cands_lhs (same lhs called twice)
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines1 = eh.candidate("git ");
        const Vector<String> lines2 = eh.candidate("git ");   // cache hit on lhs
        assert(lines1 == lines2);
    }

    ////////////////////////////////////////////////////////
    // Cache hit: cache_cands_mat (different lhs, same pattern match)
    // "git pu" and "git " both match [["git", ".*"], subcmd] with same token hash
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        // "git st" – matching SUBCMD pattern for git, first token "git"
        [[maybe_unused]] const Vector<String> lines1 = eh.candidate("git st");
        // "git " – same pattern but different lhs; if hash_mat is the same, cache_cands_mat hits
        // (This exercises the mat-cache path when different lhs leads to same matched tokens.)
        [[maybe_unused]] const Vector<String> lines2 = eh.candidate("git ");
        assert(lines1.size() == (size_t) cfg.area_height);
        assert(lines2.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // complete(): no candidates → returns lhs unchanged
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        eh.candidate("xyzzy_nonexistent_cmd ");  // populates cands (empty)
        const String result = eh.complete("xyzzy_nonexistent_cmd ");
        assert(result == "xyzzy_nonexistent_cmd ");
    }

    ////////////////////////////////////////////////////////
    // complete(): empty lhs → returns empty string
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        eh.candidate("");
        const String result = eh.complete("");
        assert(result == "");
    }

    ////////////////////////////////////////////////////////
    // complete(): single candidate (no trailing slash) → appends space
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        // "cat column_padding" → GREP → only "column_padding" matches
        eh.candidate("cat column_pad");
        const String result = eh.complete("cat column_pad");
        // Result must start with "cat " and end with "column_padding "
        assert(result.starts_with("cat "));
        assert(result.find("column_padding") != String::npos);
    }

    ////////////////////////////////////////////////////////
    // complete(): multiple candidates → returns common prefix
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        // "git " → multiple subcmd candidates starting differently
        eh.candidate("git ");
        const String result = eh.complete("git ");
        // Result should not crash and be a string starting with "git "
        assert(result.starts_with("git "));
    }

    ////////////////////////////////////////////////////////
    // complete(): single directory candidate (trailing slash)
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        // "./source" should match the "source/" directory uniquely (if it exists).
        eh.candidate("./source");
        const String result = eh.complete("./source");
        // Result should be "source/" (path only) or "./source/"
        assert(result.find("source") != String::npos);
    }

    ////////////////////////////////////////////////////////
    // columnize: area larger than number of items (all fit in one column)
    // Exercised indirectly via candidate() on a short list.
    ////////////////////////////////////////////////////////

    {
        // Use a very wide area so all candidates fit in one row.
        EditHelper eh(8, 200, cfg);
        const Vector<String> lines = eh.candidate("cat area");
        assert(lines.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // BASHCOMP: candidate("tar xvf") falls through to bashcomp
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        // "tar xvf" has no specific entry and the last token "xvf" is non-empty,
        // non-dash, not an existing file -> matches [[">>", ".*"], "bashcomp", ""].
        const Vector<String> lines = eh.candidate("tar xvf");
        assert(lines.size() == (size_t) cfg.area_height);
    }

    ////////////////////////////////////////////////////////
    // BASHCOMP: cache_cands_lhs hit on identical input
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines1 = eh.candidate("tar xvf");
        const Vector<String> lines2 = eh.candidate("tar xvf");  // lhs-cache hit
        assert(lines1 == lines2);
    }

    ////////////////////////////////////////////////////////
    // BASHCOMP: no cache_cands_mat collision across different commands
    // "pip xvf" and "tar xvf" share the same last token but must not share
    // the mat-cache, so neither call should crash or return wrong data.
    ////////////////////////////////////////////////////////

    {
        EditHelper eh(8, 80, cfg);
        const Vector<String> lines_tar = eh.candidate("tar xvf");
        const Vector<String> lines_pip = eh.candidate("pip xvf");
        // Both calls must return area_height lines without crashing.
        assert(lines_tar.size() == (size_t) cfg.area_height);
        assert(lines_pip.size() == (size_t) cfg.area_height);
    }

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main function
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void)
{   // {{{

    // Run all unit test functions.
    test_BashCompleter();
    test_AsyncComp();
    test_CharX();
    test_CmdRunner();
    test_EditHelper();
    test_ExBashConfig();
    test_GapBuffer();
    test_GenPathCache();
    test_HistManager();
    test_MimeType();
    test_PathX();
    test_preview();
    test_string_utils();
    test_TermUserIF_pty();
    test_TextEditorEmacs();
    test_TextEditorVi();
    test_tokenizers();
    test_utf8();
    test_utils();

    // Run all integration test functions.
    test_readcmd();
    test_main_exbash();
    test_main_exbash_non_tty();

    // Test run commands.
    const ExBashConfig cfg = load_config("misc/config.toml");
    print_config(cfg, "print_config=path_cmnd_info");
    print_config(cfg, "print_config=path_bash_info");

    // Print header of overall result.
    std::cout                                         << std::endl;
    std::cout << "=============================="     << std::endl;
    std::cout << "\033[33mOVERALL TEST RESULT\033[0m" << std::endl;

    // Print test result.
    if (passed) { std::cout << "\033[32mPASSED\033[0m" << std::endl; }
    else        { std::cout << "\033[31mFAILED\033[0m" << std::endl; }

    std::cout << "test finished" << std::endl;

    return EXIT_SUCCESS;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
