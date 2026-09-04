////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: history_manager.cxx                                                         ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "history_manager.hxx"

// Include STL headers.
#include <algorithm>

// Include the headers of custom modules.
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// HistManager: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

HistManager::HistManager(const Deque<String>& storage)
{   // {{{

    // Update the cache.
    for (const String& s : storage)
        this->hist_views.emplace_back(s);

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// HistManager: Member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

StringView HistManager::complete(StringView lhs) const noexcept
{   // {{{

    // Returns empty string immediately if no completion query is given.
    if (lhs.size() == 0) return StringView("");

    // The predicate function for the "std::find_if" below.
    const auto predicate = [lhs](StringView sv) { return sv.starts_with(lhs); };

    // Search the matched history from the end.
    const auto hist_iter = std::find_if(this->hist_views.rbegin(), this->hist_views.rend(), predicate);

    // Returns empty string if no matched history found.
    if (hist_iter == this->hist_views.rend())
        return StringView("");

    // Otherwise, returns the rest of the matched history.
    return hist_iter->substr(lhs.size());

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
