////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: config.cxx                                                                  ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the header.
#include "config.hxx"

// Include the headers of STL.
#include <cstdlib>
#include <iostream>

// Include the header of the toml++ library.
#define TOML_EXCEPTIONS 0
#include <toml.hpp>

// Include custom headers.
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// File-local helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    void set_config(OmniPickerConfig& cfg, const toml::table& table, const toml::key& section, const toml::key& key) noexcept
    // Set one config item to the given config instance.
    //
    // [Args]
    //   cfg     (OmniPickerConfig&) : [OUT] Config instance to update.
    //   table   (const toml::table&): [IN]  Top node of the config file.
    //   section (const String&)     : [IN]  Section name.
    //   key     (const String&)     : [IN]  Key name.
    //
    {   // {{{

        // Get target config item.
        toml::node_view node = table[section][key];

        // Read the target section.
        if (section == "PLUGIN_OMNIPICKER")
        {
            switch (hash(key))
            {
                // Configuration values for file picker.
                case hash("w1_ratio")       : cfg.w1_ratio = node.value_or(cfg.w1_ratio);               break;
                case hash("w2_ratio")       : cfg.w2_ratio = node.value_or(cfg.w2_ratio);               break;
                case hash("preview_cmd_txt"): cfg.preview_cmd_txt = node.value_or(cfg.preview_cmd_txt); break;
                case hash("preview_cmd_bin"): cfg.preview_cmd_bin = node.value_or(cfg.preview_cmd_bin); break;

                // Configuration values for history picker.
                case hash("path_history"): cfg.path_history = node.value_or(cfg.path_history); break;

                // Configuration values for process id picker.
                case hash("ps_command")   : cfg.ps_command    = node.value_or(cfg.ps_command);    break;
                case hash("idx_pid_field"): cfg.idx_pid_field = node.value_or(cfg.idx_pid_field); break;
            }
        }

    }   // }}}

    OmniPickerConfig parse_toml_file(StringView path_cfg)
    // Parse the given TOML file and return the config values.
    //
    // [Args]
    //   path_cfg (StringView): [IN] The path to the TOML config file.
    //
    // [Returns]
    //   (OmniPickerConfig): The config values loaded from the given TOML file, or default values if the file cannot be read.
    //
    {   // {{{

        OmniPickerConfig cfg;

        // Read specified TOML file.
        toml::parse_result result = toml::parse_file(path_cfg);
        if (not result)
        {
            // Prepare error message.
            std::cerr << result.error().description() << " (" << result.error().source() << ")" << std::endl;
            return cfg;
        }

        // Steal the table from the result.
        toml::table table = std::move(result).table();

        for (const auto& node_section : table)
        {
            // If the node is a table then read the values contained in the table.
            if (node_section.second.is_table())
                for (auto node_value : *node_section.second.as_table())
                    set_config(cfg, table, node_section.first, node_value.first);
        }

        return cfg;

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

OmniPickerConfig get_config(StringView path_cfg)
{   // {{{

    // List of config file paths to check, in order of priority.
    //   - priority 1: User-specified config file.
    //   - priority 2: User's config file.
    //   - priority 3: Default config file.
    Vector<String> list_path_config = {
        expand_tilde(path_cfg),
        expand_tilde("~/.config/exbash/config.toml"),
        expand_tilde("~/.local/share/exbash/default/config.toml"),
    };

    for (const String& path_config : list_path_config)
        if (stdfs::is_regular_file(path_config))
            return parse_toml_file(path_config);

    return OmniPickerConfig{};

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
