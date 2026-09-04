////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: line_model.cxx                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "line_model.hxx"

// Include custom headers.
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

LineModel::LineModel(const Vector<String>& items) : focus_filt(0), grep_str("")
{   // {{{

    Set<String> registered_items;

    // Initialize the main contents.
    for (const String& item : items)
    {
        if (not registered_items.contains(item))
        {
            this->contents_main.push_back(LineItem{item, false});
            registered_items.insert(item);
        }
    }

    for (const LineItem& item : contents_main)
        this->contents_filt.push_back(item);

    // Update the filtered contents based on the initial grep string (= empty string).
    this->update(this->grep_str);

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Member function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

const Vector<LineItem>& LineModel::get_contents_filt(void)
{ return this->contents_filt; }

int32_t LineModel::get_focus_filt(void) const
{ return this->focus_filt; }

LineItem LineModel::get_item(void) const
{   // {{{

    // Safe guard against out-of-bounds access.
    if ((this->focus_filt < 0) or (static_cast<int32_t>(this->contents_filt.size()) <= this->focus_filt))
        return LineItem{};

    return this->contents_filt[static_cast<size_t>(this->focus_filt)];

}   // }}}

Vector<String> LineModel::get_selected(void) const
{   // {{{

    Vector<String> result;

    for (const LineItem& item : this->contents_filt)
        if (item.star)
            result.push_back(item.line);

    // If there are no starred items, return the focused item.
    if (result.empty())
    {
        const LineItem item = this->get_item();
        if (not item.line.empty())
            result.push_back(item.line);
    }

    return result;

}   // }}}

String LineModel::process_key(StringView key, StringView grep_str_win)
{   // {{{

    // Update the model state.
    this->update(grep_str_win);

    // Get the number of filtered items (for bounds checking).
    const int32_t num_filt = static_cast<int32_t>(this->contents_filt.size());

    switch (hash(key))
    {
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
    }

    return this->grep_str;

}   // }}}

void LineModel::update(StringView grep_str)
{   // {{{

    // Do nothing if the path is the same as the current path.
    if (this->grep_str == grep_str)
        return;

    // Update the internal state of the filer model.
    this->grep_str = grep_str;

    // Update the filtered contents based on the grep string.
    this->contents_filt.clear();
    for (const LineItem& item : contents_main)
        if (item.line.find(this->grep_str) != String::npos)
            this->contents_filt.push_back(item);

    // Reset the focus index to 0 (top of the filtered list).
    this->focus_filt = 0;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
