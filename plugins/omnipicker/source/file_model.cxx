////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: file_model.cxx                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "file_model.hxx"

// Include custom headers.
#include "file_preview.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// File-local helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    String to_relpath(const Path& path)
    // Return path relative to the current working directory (or the absolute path on failure).
    //
    // [Args]
    //   path (const Path&): [IN] The input path.
    //
    // [Returns]
    //   (String): The path string relative to the current working directory, or the absolute path
    //
    {   // {{{

        std::error_code ec;
        const Path rel = stdfs::relative(path, stdfs::current_path(), ec);
        return ec ? path.string() : rel.string();

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

FileModel::FileModel(Path path_ini, const OmniPickerConfig& cfg)
    : path(""), focus_filt(0), showdot(false), grep_str(""), cfg(cfg)
{   // {{{

    this->update(path_ini, this->showdot, this->grep_str);

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////////////////////////////////////////////////

const Vector<FileItem>& FileModel::get_contents_filt(void)
{ return this->contents_filt; }

const Vector<FileItem>& FileModel::get_contents_prev(void)
{ return this->contents_prev; }

int32_t FileModel::get_focus_filt(void) const
{ return this->focus_filt; }

int32_t FileModel::get_focus_prev(void) const
{   // {{{

    const String target_name = path.filename().string();

    int32_t focus_prev = 0;
    for (int i = 0; i < static_cast<int>(this->contents_prev.size()); i++)
        if (contents_prev[i].left == target_name) { focus_prev = i; break; }

    return focus_prev;

}   // }}}

FileItem FileModel::get_item(void) const
{   // {{{

    // Safe guard against out-of-bounds access.
    if ((this->focus_filt < 0) or (static_cast<int32_t>(this->contents_filt.size()) <= this->focus_filt))
        return FileItem{};

    return this->contents_filt[static_cast<size_t>(this->focus_filt)];

}   // }}}

Vector<String> FileModel::get_preview(const String& name, int32_t max_height)
{   // {{{

    // Normalize the path to avoid issues with redundant components (e.g., "./", "../").
    const Path path_norm = (this->path / name).lexically_normal();

    // Generate the preview if it is not cached.
    const String key = path_norm.string();
    if (not this->preview_cache.contains(key))
        this->preview_cache[key] = preview(path_norm, max_height, this->cfg);

    return this->preview_cache[key];

}   // }}}

Vector<String> FileModel::get_selected(void) const
{   // {{{

    Vector<String> result;

    for (const FileItem& item : this->contents_filt)
        if (item.star)
            result.push_back(to_relpath(path / item.left));

    // If there are no starred items, return the focused item.
    if (result.empty())
    {
        const FileItem item = this->get_item();
        if (not item.left.empty())
            result.push_back(to_relpath(path / item.left));
    }

    return result;

}   // }}}

String FileModel::process_key(StringView key, StringView grep_str_win)
{   // {{{

    // Update the model state.
    this->update(this->path, this->showdot, grep_str_win);

    // Get the number of filtered items (for bounds checking).
    const int32_t num_filt = static_cast<int32_t>(this->contents_filt.size());

    switch (hash(key))
    {
        // Toggle display of hidden (dot) files.
        case hash("."):
            this->update(this->path, not this->showdot, this->grep_str); break;

        // Cursor movement.
        // NOTE: clip(..., 0, num_filt - 1) works even if num_filt is 0.
        case hash("k"):
        case hash("KEY_UP")  : this->focus_filt = clip(this->focus_filt -  1, 0, num_filt - 1); break;
        case hash("j"):
        case hash("KEY_DOWN"): this->focus_filt = clip(this->focus_filt +  1, 0, num_filt - 1); break;
        case hash("^B")      : this->focus_filt = clip(this->focus_filt - 10, 0, num_filt - 1); break;
        case hash("^F")      : this->focus_filt = clip(this->focus_filt + 10, 0, num_filt - 1); break;
        case hash("0")       : this->focus_filt = 0;                                            break;
        case hash("G")       : this->focus_filt = std::max(0, num_filt - 1);                    break;

        // Select/deselect the focused item and move focus down.
        case hash(" "):
            if (not this->contents_filt.empty())
            {
                this->contents_filt[static_cast<size_t>(this->focus_filt)].star ^= true;
                this->focus_filt = clip(this->focus_filt + 1, 0, num_filt - 1);
            }
            break;

        case hash("-"):
        case hash("h"):
        case hash("KEY_LEFT"):
            this->update(this->path.parent_path(), this->showdot, "");
            break;

        case hash("l"):
        case hash("KEY_RIGHT"):
            if (not this->contents_filt.empty())
            {
                // Move to the directory if the focused item is a directory.
                Path path_target = this->path / this->get_item().left;
                if (stdfs::is_directory(path_target))
                    this->update(path_target, this->showdot, "");
            }
            break;
    }

    return this->grep_str;

}   // }}}

void FileModel::update(const Path& path, bool showdot, StringView grep_str)
{   // {{{

    // Do nothing if the path is the same as the current path.
    if ((this->path == path) and (this->showdot == showdot) and (this->grep_str == grep_str))
        return;

    // Update the internal state of the filer model.
    this->path     = path;
    this->showdot  = showdot;
    this->grep_str = grep_str;

    // Update the main and previous directory contents.
    this->contents_main = listdir(path,               this->showdot, true);
    this->contents_prev = listdir(path.parent_path(), this->showdot, false);

    // Update the filtered contents based on the grep string.
    this->contents_filt.clear();
    for (const FileItem& item : contents_main)
        if (item.left.find(this->grep_str) != String::npos)
            this->contents_filt.push_back(item);

    // Reset the focus index to 0 (top of the filtered list).
    this->focus_filt = 0;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
