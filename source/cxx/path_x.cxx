////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: path_x.cxx                                                                  ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "path_x.hxx"

// Include the headers of custom modules.
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// PathX: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

PathX::PathX(const stdfs::path& path) : stdfs::path(path)
{ /* Do nothing, initializer lists only. */ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// PathX: Member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<String> PathX::listdir(uint32_t n_max_items) const
{   // {{{

    // Initilize returned vector.
    Vector<String> result;

    // Empty path will be regarded as a current directory.
    PathX target = (not this->empty()) ? *this : PathX("./");

    // Replace "~" to home directory.
    const char* home = getenv("HOME");
    if ((not target.empty()) and target.native().starts_with("~") and (home != nullptr))
    {
        String target_str = target.string();
        if (target_str.size() >= 1)
            target = PathX(home + target_str.substr(1));
    }

    // Returns empty vector if the target directory does not exist.
    if (not stdfs::is_directory(target))
        return result;

    // Collect all files and directories inside the target directory.
    std::error_code ec;
    for (const auto& entry : stdfs::directory_iterator(target, stdfs::directory_options::skip_permission_denied))
    {
        // Get relative path.
        const auto path_relative = entry.path().lexically_relative(target);

        //
        bool is_dir = entry.is_directory(ec);
        if (ec) is_dir = false;

        // Add to the returned vector.
        result.push_back(path_relative.string() + (is_dir ? "/" : ""));

        // Stop directory search if exceeds the maximum number of items.
        // Because it takes too long time for searching a big directory and
        // it tend to prevent user's comfortable command editing experience.
        if (result.size() >= n_max_items)
            break;
    }

    // Define sorting key function.
    // The sorting rule is the same as `ls --group-directories-first` command.
    constexpr auto sort_key_func = [](const String& s1, const String& s2) noexcept -> bool
    {
        const bool is_dir1 = (s1.size() > 0) and (s1.back() == '/');
        const bool is_dir2 = (s2.size() > 0) and (s2.back() == '/');

        if      (is_dir1 and is_dir2) return (s1 < s2);
        else if (is_dir1            ) return true;
        else if (            is_dir2) return false;
        else                          return (s1 < s2);
    };

    // Sort list results.
    std::sort(result.begin(), result.end(), sort_key_func);

    return result;

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Tuple<PathX, String> split_to_target_and_query(const Vector<StringView>& tokens)
{   // {{{

    // Initialize the target path.
    PathX basepath = PathX("");

    // Update the base path if the token is not empty.
    if (tokens.size() > 0)
        basepath = PathX(strip(tokens.back()));

    // Split the target token to parent path and file name.
    return {PathX(basepath.parent_path()), basepath.filename().string()};

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
