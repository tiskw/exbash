////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: preview.hxx                                                                 ///
///                                                                                              ///
/// This file defines the function `preview` that returns preview result as a vector of string.  ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PREVIEW_HXX
#define PREVIEW_HXX

// Include STL headers.
#include <cstdint>

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<String> preview(StringView path, uint16_t height, const StringMap& previews);
// Returns preview contents of the given file.
//
// [Args]
//   path     (StringView)      : [IN] Path to the target file.
//   height   (int16_t)         : [IN] Height of preview window.
//   previews (const StringMap&): [IN] Map of the preview commands.
//
// [Returns]
//   (const Vector<String>&): Lines of preview contents.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
