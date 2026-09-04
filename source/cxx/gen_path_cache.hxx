////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: gen_path_cache.hxx                                                          ///
///                                                                                              ///
/// Generate the cache of available command names in the directories specified in the "PATH"     ///
/// environment variable.                                                                        ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CMDS_CACHE_HXX
#define CMDS_CACHE_HXX

// Include the headers of custom modules.
#include "config.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

int32_t generate_path_commands_cache(const ExBashConfig& cfg);
// Generate the command cache by searching all executable files in the directories specified
// in the "PATH" environment variable, and save it to "config.path_cmnd_info" file.
//
// [Returns]
//   (int32_t): EXIT_SUCCESS if no error occurred, otherwise EXIT_FAILURE.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
