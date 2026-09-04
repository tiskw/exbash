////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: edit_helper.hxx                                                             ///
///                                                                                              ///
/// This file defines the class `EditHelper` which will be used for computing command/path       ///
/// completion candidates, and present them to users.                                            ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef EDIT_HELPER_HXX
#define EDIT_HELPER_HXX

// Include STL headers.
#include <future>

// Include the headers of custom modules.
#include "bash_completer.hxx"
#include "carapace_service.hxx"
#include "config.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////////////////////////////////////////////

class EditHelper
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

        EditHelper(uint16_t rows, uint16_t cols, const ExBashConfig& cfg);
        // Default constructor of EditHelper.
        //
        // [Args]
        //   rows (uint16_t)           : [IN] Height of completion area.
        //   cols (uint16_t)           : [IN] Width of completion area.
        //   cfg  (const ExBashConfig&): [IN] Config data.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        Vector<String> candidate(StringView lhs);
        // Returns candidates of completion.
        //
        // [Args]
        //   lhs (StringView): [IN] Left-hand-side of the user input.
        //
        // [Returns]
        //   (Vector<String>): Array of lines (strings) for showing completion candidates to users.

        String complete(StringView lhs) const;
        // Execute completion.
        // This function should be called after calling `candidate` function.
        //
        // [Args]
        //   lhs (StringView): [IN] Left-hand-side of the user input.
        //
        // [Returns]
        //   (String): Completed left-hand-side string.

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        Size area_size;
        // Size of the drawing area.

        BashCompleter bash_completer;
        // An instance of BashCompleter for computing completion candidates from "bash-complete".

        CarapaceService carapace_service;
        // An instance of CarapaceService for computing completion candidates from "carapace".

        Vector<String> cache_commands;
        // Cache of available command names.

        Vector<Pair<String, String>> cands;
        // Completion candidates.

        CandCacheMap cache_cands_lhs;
        CandCacheMap cache_cands_mat;
        // Cache of completion candidates, where the key is the hash value of the left-hand-side
        // string or the hash value of the matched token of the completion patterns.

        Vector<String> lines;
        // Completion lines.

        Map<String, Vector<Tuple<String, String>>> opt_cache;
        // Cache of the command options.

        StrVecMap subcmd_cache;
        // Cache of the command options.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Static member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        static std::shared_future<Vector<String>> shared_future_cache_commands;
        // Shared future instance of the command cache.

        static std::shared_future<Vector<Vector<RegEx>>> shared_future_vec_patterns_regex;
        // Cache of compiled regular expression patterns for completion matching.

        static std::once_flag flag_init_shared_futures;
        // A flag for one-time initialization of shared future instances.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Copied values from the config data.
        ////////////////////////////////////////////////////////////////////////////////////////////

        const uint16_t column_padding;
        // Margin of column display in the completion list.

        const Vector<Completion> completions;
        // Completion patterns and their types and optional strings.

        const StringMap previews;
        // Collection of MIME type and its preview command.

        const String preview_delim;
        // Delimiter of the preview window.

        const float preview_ratio;
        // Width of the preview window.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        Vector<String> candidate_from_cache(const CandCacheMap& cache, uint64_t hash_key);
        // Get completion lines from the cache.
        //
        // [Args]
        //   cache    (const CandCacheMap&): [IN] Cache of completion candidates.
        //   hash_key (uint64_t)           : [IN] Hash key for the cache.
        // 
        // [Returns]
        //   (Vector<String>): Array of lines (strings) for showing completion candidates to users.

        void cands_bashcomp(StringView lhs, const Vector<StringView>& tokens);
        // Compute completion candidates from bash-completion.
        //
        // [Args]
        //   lhs    (StringView)              : [IN] Left-hand-side of the user input.
        //   tokens (const Vector<StringView>): [IN] Parsed tokens of the user input.

        void cands_command(const Vector<StringView>& tokens, const String& option);
        // Compute command completion candidates.
        // The result candidates will be stored in `this->cands`.
        //
        // [Args]
        //   tokens (const Vector<StringView>): [IN] Parsed tokens of the user input.
        //   option (const String&)           : [IN] Optional string of the completion.

        void cands_carapace(const Vector<StringView>& tokens);
        // Compute completion candidates from carapace.
        //
        // [Args]
        //   tokens (const Vector<StringView>): [IN] Parsed tokens of the user input.

        void cands_filepath(const Vector<StringView>& tokens);
        // Compute file path completion candidates.
        // The result candidates will be stored in `this->cands`.
        //
        // [Args]
        //   tokens (const Vector<StringView>): [IN] Parsed tokens of the user input.

        void cands_grep(const Vector<StringView>& tokens, const String& option);
        // Compute command option candidates from grep.
        //
        // [Args]
        //   tokens (const Vector<StringView>): [IN] Parsed tokens of the user input.
        //   option (const String&)           : [IN] Optional string (tab-separated string).

        void cands_option(const Vector<StringView>& tokens);
        // Compute command option candidates.
        // The result candidates will be stored in `this->cands`.
        //
        // [Args]
        //   tokens (const Vector<StringView>): [IN] Parsed tokens of the user input.

        void cands_preview(const Vector<StringView>& tokens);
        // Compute completion candidates for preview.
        //
        // [Args]
        //   tokens (const Vector<StringView>): [IN] Parsed tokens of the user input.

        void cands_shell(const Vector<StringView>& tokens, const String& option);
        // Compute completion candidates from shell command.
        //
        // [Args]
        //   tokens (const Vector<StringView>): [IN] Parsed tokens of the user input.
        //   option (const String&)           : [IN] Optional string (normally it is a shell command).

        void cands_subcmd(const Vector<StringView>& tokens, const String& option);
        // Compute completion candidates from sub command.
        //
        // [Args]
        //   tokens (const Vector<StringView>): [IN] Parsed tokens of the user input.
        //   option (const String&)           : [IN] Optional string.

        void cands_sc_and_bc(StringView lhs, const Vector<StringView>& tokens, const String& option);
        // Combination of sub command and bash-completion candidates.
        //
        // [Args]
        //   lhs    (StringView)              : [IN] Left-hand-side of the user input.
        //   tokens (const Vector<StringView>): [IN] Parsed tokens of the user input.
        //   option (const String&)           : [IN] Optional string.

        void lines_from_cands(const Vector<Pair<String, String>>& cands);
        // Convert candidate to strings that will be shown to users.
        //
        // [Args]
        //   cands (const Vector<Pair<String, String>>&): [IN] Completion candidates.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Static private functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        static void init_shared_futures(const ExBashConfig& cfg);
        // Initialize shared future instances for caching command names and compiled regular expression patterns.
        //
        // [Args]
        //   cfg (const ExBashConfig&): [IN] Config data for initializing the shared futures.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
