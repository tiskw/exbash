////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: grid_model.cxx                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "grid_model.hxx"

// Include custom headers.
#include "utils.hxx"


////////////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

GridModel::GridModel(const Vector<String>& items) : LineModel(items)
{   // {{{

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Member function implementations
////////////////////////////////////////////////////////////////////////////////////////////////////

String GridModel::process_key(StringView key, StringView grep_str_win, Tuple<int32_t, int32_t> grid_shape)
{   // {{{

    // Update the model state.
    this->update(grep_str_win);

    // Get the number of filtered items (for bounds checking).
    const int32_t num_filt = static_cast<int32_t>(this->contents_filt.size());

    // Unpack the grid shape into rows and columns.
    const auto [num_rows, num_cols] = grid_shape;

    // Compute the cursor increment levels.
    const int32_t delta0 = 1;
    const int32_t delta1 = num_cols;
    const int32_t delta2 = num_cols * num_rows;

    switch (hash(key))
    {
        // Cursor movement.
        // NOTE: clip(..., 0, num_filt - 1) works even if num_filt is 0.
        case hash("h"):
        case hash("KEY_LEFT") : this->focus_filt = clip(this->focus_filt - delta0, 0, num_filt - 1); break;
        case hash("l"):
        case hash("KEY_RIGHT"): this->focus_filt = clip(this->focus_filt + delta0, 0, num_filt - 1); break;
        case hash("k"):
        case hash("KEY_UP")   : this->focus_filt = clip(this->focus_filt - delta1, 0, num_filt - 1); break;
        case hash("j"):
        case hash("KEY_DOWN") : this->focus_filt = clip(this->focus_filt + delta1, 0, num_filt - 1); break;
        case hash("^B")       : this->focus_filt = clip(this->focus_filt - delta2, 0, num_filt - 1); break;
        case hash("^F")       : this->focus_filt = clip(this->focus_filt + delta2, 0, num_filt - 1); break;
        case hash("0")        : this->focus_filt = 0;                                                break;
        case hash("G")        : this->focus_filt = std::max(0, num_filt - 1);                        break;

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

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
