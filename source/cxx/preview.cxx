////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: preview.cxx                                                                 ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "preview.hxx"

// Include STL headers.
#include <fstream>

// Include POSIX headers.
#include <fnmatch.h>

// Include the headers of custom modules.
#include "cmd_runner.hxx"
#include "mime_type.hxx"
#include "string_utils.hxx"
#include "tokenizers.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    String get_default_preview(StringView mime_type, StringView path)
    // Get the default preview output for the given file path based on the file type.
    //
    // [Args]
    //   mime_type (StringView): [IN] File type string of the preview target.
    //   path      (StringView): [IN] File path of the preview target.
    //
    // [Returns]
    //   (String): Preview output string.
    //
    {   // {{{

        // Number of bytes to read from the file for previewing.
        constexpr int32_t bytes_to_read = 1024;

        // Case 1: directory.
        if (mime_type == "inode/directory")
            return "";

        // Case 2: text file.
        else if (mime_type.starts_with("text/") or (mime_type == "application/x-sh"))
        {
            // Open the file in binary mode to read raw bytes without any encoding conversion.
            std::ifstream ifs(String(path), std::ios::binary);
            if (not ifs) return "";

            // Read the first `bytes_to_read` bytes from the file into a buffer.
            String buffer(bytes_to_read, '\0');
            ifs.read(buffer.data(), bytes_to_read);
            return buffer;
        }

        // Case 3: binary file.
        else
        {
            String result;
            result += "<Binary file>\n";
            result += std::format("  Size: {:L} bytes", stdfs::file_size(path));
            return result;
        }

        return "";

    }   // }}}

    String get_user_preview(StringView command, StringView path)
    // Get the preview output by running the user-defined command.
    //
    // [Args]
    //   command (StringView): [IN] User-defined command string for previewing.
    //   path    (StringView): [IN] File path of the preview target.
    //
    // [Returns]
    //   (String): Preview output string.
    //
    {   // {{{

        // Get command to be run.
        String command_full = replace(command, "{path}", std::format("'{}'", path));

        // Tokenize the command string.
        Vector<StringView> cmd_tokens;
        for (const StringView token : tokenize(command_full, TOKENIZE_DEQUOTE))
            cmd_tokens.emplace_back(token);

        // Get command output, and replace TAB to 4 white spaces.
        return replace(run_command(cmd_tokens, RUN_COMMAND_GETOUT), "\t", "    ");

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<String> preview(StringView path, uint16_t height, const StringMap& previews)
{   // {{{

    // Instantiate a class to get mime type of a file.
    static MimeType mime_type;

    // Create instance for return value.
    Vector<String> result;

    // Do nothing if the target file path does not exist.
    if (not stdfs::exists(Path(path)))
        return result;

    // Compute file type string of the preview target.
    const String mime_type_target = mime_type.get(path);

    // Get the preview command if the matched preview pattern found.
    StringView command;
    for (const auto& [mime_type_pattern, cmd] : previews)
        if (fnmatch(mime_type_pattern.c_str(), mime_type_target.c_str(), 0) == 0)
            command = StringView(cmd);

    // Get the preview output.
    const String output = (command.empty()) ? get_default_preview(mime_type_target, path) : get_user_preview(command, path);

    // Do nothing if the preview output is empty.
    if (output.empty()) return result;

    // Split command output into lines.
    for (const StringView line : split(output, "\n"))
    {
        result.emplace_back(line);

        // Preview result is limited to the given height.
        if (result.size() >= height)
            break;
    }

    // Exit function if one preview procedure finished.
    return result;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
