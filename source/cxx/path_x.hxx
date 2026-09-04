////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: path_x.hxx                                                                  ///
///                                                                                              ///
/// This file defines the class `PathX` that provides utility functions related to file path.    ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PATH_X_HXX
#define PATH_X_HXX

// Include STL headers.
#include <filesystem>

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////////////////////////////////////////////

class PathX : public std::filesystem::path
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        explicit PathX(const std::filesystem::path& path);
        // Constructor of PathX.
        //
        // [Args]
        //   path (const std::filesystem::path&): [IN] Source path to be copied.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        Vector<String> listdir(uint32_t n_max_items = 128) const;
        // Returns a list of names of the entries in the given directory path.
        // The list is sorted in ascending order.
        //
        // [Args]
        //   n_max_items (uint32_t): [IN] The maximum number of items to be listed.
        //
        // [Returns]
        //   (Vector<string>): List of names of the entries.
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Tuple<PathX, String> split_to_target_and_query(const Vector<StringView>& tokens);
// Split the path to completion target and query.
//
// [Args]
//   tokens (const Vector<StringView>&): [IN] User input tokens.
//
// [Returns]
//   (PathX) : Completion target.
//   (String): Completion query.
//

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
