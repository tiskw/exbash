////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: file_preview.cxx                                                            ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the header.
#include "file_preview.hxx"

// Include the headers of STL.
#include <sstream>

// Include POSIX headers.
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

// Include custom headers.
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// File-local helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    String get_binary_file_info(const Path& path)
    // Get a string containing information about a binary file, such as its size.
    //
    // [Args]
    //   path (const Path&): [IN] The file path of the binary file.
    //
    // [Returns]
    //   (String): A string containing information about the binary file.
    //
    {   // {{{

        std::error_code ec;
        const auto size = stdfs::file_size(path, ec);

        if (ec)
            return "<Binary file>\n  Size: <unknown>\n(error: " + ec.message() + ")\n";

        String result;
        result += "<Binary file>\n";
        result += "  Size: " + std::to_string(size) + " bytes\n";

        return result;

    }   // }}}

    String replace_placeholder(const String& tmpl, const String& path_str)
    // Replace the literal "{path}" placeholder in a template string with the given path.
    //
    // [Args]
    //   tmpl     (const String&): [IN] The template string containing the "{path}" placeholder.
    //   path_str (const String&): [IN] The string to replace the "{path}" placeholder with.
    //
    // [Returns]
    //   (String): The resulting string after replacement.
    //
    {   // {{{

        // Make a copy of the template string to modify.
        String result = tmpl;

        // Find the first occurrence of "{path}" and replace it with path_str.
        size_t pos;
        while ((pos = result.find("{path}")) != String::npos)
            result.replace(pos, 6, shlex_quote(path_str));

        return result;

    }   // }}}

    Vector<String> split_lines_raw(const String& text, int32_t max_lines)
    // Split a string into a vector of lines.
    //
    // [Args]
    //   text      (const String&): [IN] The input string to split.
    //   max_lines (int32_t)      : [IN] The maximum number of lines to return.
    //
    // [Returns]
    //   (Vector<String>): A vector of lines obtained by splitting the input string.
    //
    {   // {{{

        // Use a stringstream to split the input text into lines.
        Vector<String> lines;

        // std::istringstream allows us to read the string line by line using std::getline.
        std::istringstream ss(text);

        // Read each line from the stringstream and add it to the vector.
        String line;
        while (std::getline(ss, line) and (static_cast<int32_t>(lines.size()) < max_lines))
            lines.push_back(line);

        return lines;

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<FileItem> listdir(const Path& dirpath, bool showdot, bool extra, int32_t max_items)
{   // {{{

    using EntryData = std::tuple<String, Vector<String>, int, bool>;
    Vector<EntryData> entries;

    // Do nothing if the directory does not exist or is not a directory.
    std::error_code ec;
    if (not stdfs::is_directory(dirpath, ec))
        return {};

    for (const stdfs::directory_entry& entry : stdfs::directory_iterator(dirpath, ec))
    {
        // Stop if we have reached the maximum number of items.
        if (static_cast<int32_t>(entries.size()) >= max_items) break;

        // Skip hidden files if showdot is false.
        const String name = entry.path().filename().string();
        if ((not showdot) and (not name.empty()) and (name[0] == '.'))
            continue;

        // Get the file status.
        struct stat st;
        if (stat(entry.path().c_str(), &st) != 0)
            continue;

        // Determine if the entry is a directory.
        const bool is_dir = S_ISDIR(st.st_mode);

        // Choose curses color attribute.
        int32_t attr;
        if      (is_dir)                                     attr = COLOR_PAIR(4);  // blue
        else if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) attr = COLOR_PAIR(2);  // green
        else                                                 attr = A_NORMAL;

        // Prepare extra info if requested.
        Vector<String> extra_info;
        if (extra)
        {
            // Get the owner and group names from the UID and GID.
            const struct passwd* pw = ::getpwuid(st.st_uid);
            const struct group*  gr = ::getgrgid(st.st_gid);

            // Format the mtime to a string in the format "YYYY/MM/DD HH:MM:SS".
            char timebuf[32];
            struct tm* tm_info = localtime(&st.st_mtime);
            strftime(timebuf, sizeof(timebuf), "%Y/%m/%d %H:%M:%S", tm_info);

            extra_info = {
                get_perm_str(st.st_mode),
                pw ? pw->pw_name : std::to_string(st.st_uid),
                gr ? gr->gr_name : std::to_string(st.st_gid),
                get_size_str(static_cast<uint64_t>(st.st_size)),
                String(timebuf),
            };
        }

        entries.emplace_back(name, std::move(extra_info), attr, is_dir);
    }

    // Return an empty vector if there are no entries,
    // because the following procedure is not necessary for an empty directory.
    if (entries.empty()) return {};

    // Compute maximum field widths for right-side alignment.
    Vector<SizeType> maxlen;
    if (extra)
    {
        // Initialize the maxlen vector with the size of the extra info fields.
        maxlen.assign(std::get<1>(entries[0]).size(), 0);

        // Iterate over each entry and update the maximum length for each field in the extra info.
        for (const auto& [n, ei, a, d] : entries)
            for (SizeType i = 0; i < ei.size() && i < maxlen.size(); i++)
                maxlen[i] = std::max(maxlen[i], ei[i].size());
    }

    // Build FileItem vectors, separating directories from regular files.
    Vector<FileItem> items_dir, items_reg;

    // Pad the right-side strings for each entry and classify them into directories and regular files.
    for (const auto& [name, ei, attr, is_dir] : entries)
    {
        String right;
        if (extra and not ei.empty())
            for (size_t i = 0; i < ei.size(); i++)
                right += (i > 0 ? " " : "") + pad_right(ei[i], maxlen[i]);

        FileItem item;
        item.left  = name;
        item.right = right;
        item.attr  = attr;
        (is_dir ? items_dir : items_reg).push_back(std::move(item));
    }

    // Sort the directories and regular files separately.
    std::sort(items_dir.begin(), items_dir.end());
    std::sort(items_reg.begin(), items_reg.end());

    // Concatenate the sorted directories and regular files into a single result vector.
    Vector<FileItem> result;
    result.insert(result.end(), std::make_move_iterator(items_dir.begin()), std::make_move_iterator(items_dir.end()));
    result.insert(result.end(), std::make_move_iterator(items_reg.begin()), std::make_move_iterator(items_reg.end()));
    return result;

}   // }}}

Vector<String> preview(const Path& path, int height, const OmniPickerConfig& cfg)
{   // {{{

    // Case 0: path does not exist - return empty vector.
    if (not stdfs::exists(path))
        return {};

    // Case 1: directory - show its contents as FileItem.
    if (stdfs::is_directory(path))
    {
        // Get the list of items in the directory and convert them to a vector of strings.
        Vector<String> lines;
        for (const FileItem& item : listdir(path, false, false, height))
            lines.push_back(item.left);
        return lines;
    }

    // Case 2: text file.
    if (is_text_file(path))
    {
        // Case 2.1: no preview command configured.
        if (cfg.preview_cmd_txt.empty())
            return split_lines_raw(read_file(path, 1024 * 9), height);

        // Case 2.2: preview command configured.
        return split_lines_raw(check_output(replace_placeholder(cfg.preview_cmd_txt, path.string())), height);
    }

    // Case 3: binary file.
    else
    {
        // Case 3.1: no preview command configured.
        if (cfg.preview_cmd_bin.empty())
            return split_lines_raw(get_binary_file_info(path), height);

        // Case 3.2: preview command configured.
        return split_lines_raw(check_output(replace_placeholder(cfg.preview_cmd_bin, path.string())), height);
    }

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
