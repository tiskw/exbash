////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: config.hxx                                                                  ///
///                                                                                              ///
/// Define a config class and functions to load config from a TOML file.                         ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_HXX
#define CONFIG_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Data type declaration
////////////////////////////////////////////////////////////////////////////////////////////////////

struct ExBashConfig
{
    ////////////////////////////////////////////////////////////////////////////
    // General settings.
    ////////////////////////////////////////////////////////////////////////////

    // Height of the drawing area.
    uint16_t area_height = 6;

    // Margin of column display in the completion list.
    uint16_t column_padding = 3;

    // Path to history file.
    String path_history = "~/.bash_history";

    // The maximum number of histories to read.
    uint16_t max_hist_size = 1000;

    // Prefix and postfix strings of the datetime string.
    String datetime_pre  = "[";
    String datetime_post = "] ";

    // Prefix and postfix strings of the command history completions.
    String histhint_pre  = "";
    String histhint_post = "";

    // Horizontal line character and its color.
    String hline_char  = "-";
    String hline_color = "";

    // Path to info files.
    String path_cmnd_info = "/tmp/exbash/cmnd_info.txt";
    String path_bash_info = "/tmp/exbash/bash_info.txt";

    // Path to plugin output file.
    String output_plugin = "/dev/shm/exbash/plugin.out";

    ////////////////////////////////////////////////////////////////////////////
    // Prompt strings.
    ////////////////////////////////////////////////////////////////////////////

    // Zero-th prompt string (left and right).
    String ps0l = "";
    String ps0r = "";

    // First prompt string when insert mode.
    String ps1i = "=>> ";

    // First prompt string when normal mode.
    String ps1n = "<<= ";

    // Second prompt string.
    String ps2 = "... ";

    ////////////////////////////////////////////////////////////////////////////
    // Keybind settings.
    ////////////////////////////////////////////////////////////////////////////

    // Map of input key and command.
    StringMap keybinds;

    ////////////////////////////////////////////////////////////////////////////
    // Completion settings.
    ////////////////////////////////////////////////////////////////////////////

    // Completion patterns and their types and optional strings.
    Vector<Completion> completions = {
        {{"[./~].*"},        CompType::PATH,    ""},
        {{".+",},            CompType::COMMAND, ""},
        {{">>", "-.*"},      CompType::OPTION,  ""},
        {{">>", "FILE", ""}, CompType::PREVIEW, ""},
        {{">>", ".*"},       CompType::PATH,    ""},
    };

    ////////////////////////////////////////////////////////////////////////////
    // Preview settings.
    ////////////////////////////////////////////////////////////////////////////

    // Collection of MIME type and its preview command.
    // The {path} in the command strings will be replaced to the path to the file
    // to be previewed.
    //
    // NOTE: DO NOT USE double quote characters (") inside the command. Please
    //       use single quote (') instead.
    StringMap previews = {
        {"audio/*", "timeout 0.1s file '{path}'"},
        {"image/*", "timeout 0.1s file '{path}'"},
        {"video/*", "timeout 0.1s file '{path}'"},
    };

    // Delimiter of the preview window.
    String preview_delim = " │ ";

    // Width of the preview window.
    float preview_ratio = 0.45;

    ////////////////////////////////////////////////////////////////////////////
    // Color settings.
    ////////////////////////////////////////////////////////////////////////////

    Map<String, String> colors = {
        {"R", "\x1B[38;2;204;102;102m"},  // Red
        {"G", "\x1B[38;2;181;189;104m"},  // Green
        {"Y", "\x1B[38;2;240;198;116m"},  // Yellow
        {"B", "\x1B[38;2;129;162;190m"},  // Blue
        {"M", "\x1B[38;2;178;148;187m"},  // Magenta
        {"C", "\x1B[38;2;138;190;183m"},  // Cyan
        {"A", "\x1B[38;2;197;200;198m"},  // Gray
        {"-", "\x1B[0m"},                 // Reset
    };
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////////////////////////

ExBashConfig load_config(StringView path_cfg);
// Load config TOML file.
//
// [Args]
//   path_cfg (StringView): [IN] Path to config file.
//
// [Returns]
//   (ExBashConfig): Config instance.

int32_t print_config(const ExBashConfig& cfg, StringView arg);
// Print config value.
//
// [Args]
//   cfg (ExBashConfig): [IN] Config instance to read.
//   arg (StringView)  : [IN] Name of config value.
//
// [Returns]
//   (int32_t): Exit code.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
