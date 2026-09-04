////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: history_manager.hxx                                                         ///
///                                                                                              ///
/// This file defines the class `HistManager` which manages history file.                        ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef HISTORY_MANAGER_HXX
#define HISTORY_MANAGER_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////////////////////////////////////////////

class HistManager
// This class manages history file and provides completion results for the user input.
//
// [Notes]
//   The elements of the "hists" instance passed to the constructor of this class should NOT be
//   deleted or modified while this class is active, because this class keeps only views of
//   the string instances.
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        explicit HistManager(const Deque<String>& hists);
        // Default constructor of HistManager.
        //
        // [Args]
        //   hists (const Deque<String>&): Source of histories.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        StringView complete(StringView lhs) const noexcept;
        // Returns completion result.
        // Note that the `lhs` is not contained in the returned value.
        //
        // [Args]
        //   lhs (StringView): Left-hand-side of the editing buffer, i.e. completion query.
        //
        // [Returns]
        //   (StringView): Completion result where `lhs` itself is not contained in the result.

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        Vector<StringView> hist_views;
        // A vector of history views.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
