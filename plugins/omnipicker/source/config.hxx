////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: config.hxx                                                                  ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_HXX
#define CONFIG_HXX

// Include custom headers.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Data structures
////////////////////////////////////////////////////////////////////////////////////////////////////

struct OmniPickerConfig
// Config values for omnipicker, loaded from a TOML file.
{
    //--------------------------------------------------------------------------
    // General configuration values
    //--------------------------------------------------------------------------

    Colors colors = {{
        { 55,  59,  65}, // Black
        {204, 102, 102}, // Red
        {181, 189, 104}, // Green
        {240, 198, 116}, // Yellow
        {135, 175, 215}, // Blue
        {178, 148, 187}, // Magenta
        {138, 190, 183}, // Cyan
        {197, 200, 198}, // White
    }};
    // Color values used in the TUI window (RGB values in the range [0, 255]).
    // This will be used when "has_colors()" and "can_change_color()" return true in ncurses.

    //--------------------------------------------------------------------------
    // Configuration values for file picker
    //--------------------------------------------------------------------------

    float w1_ratio = 0.20f;
    float w2_ratio = 0.45f;
    // The width ratios for the left two panels in the file chooser window.

    String preview_cmd_txt = "";
    // Command to preview a text file ({path} is replaced with the file path).

    String preview_cmd_bin = "";
    // Command to preview a binary file.

    //--------------------------------------------------------------------------
    // Configuration values for history picker
    //--------------------------------------------------------------------------

    String path_history = "~/.bash_history";
    // Path to the history file.

    //--------------------------------------------------------------------------
    // Configuration values for process id picker
    //--------------------------------------------------------------------------

    String ps_command = "ps aux";
    // Process command for listing processes.

    int32_t idx_pid_field = 1;
    // The index of PID field in the output of "ps_command" above.
    // Note: The index is 0-based, so the default value of 1 corresponds to the second field.

};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

OmniPickerConfig get_config(StringView path_cfg);
// Load config file.
//
// [Args]
//   path_cfg (StringView): [IN] The path to the TOML config file.
//
// [Returns]
//   (OmniPickerConfig): The config values loaded from the config file.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
