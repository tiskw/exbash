////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: carapace_service.cxx                                                        ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "carapace_service.hxx"

// Include the header of JSON library.
#include "json.hpp"
using json = nlohmann::json;

// Include the headers of custom modules.
#include "cmd_runner.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CarapaceService: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

CarapaceService::CarapaceService(void)
{ /* Do nothing. */ }

CarapaceService::~CarapaceService(void)
{ /* Do nothing. */ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// CarapaceService: Member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Generator<Pair<StringView, StringView>> CarapaceService::complete(const Vector<StringView>& tokens)
{   // {{{

    constexpr auto sort_func = [](const Pair<StringView, StringView>& p1, const Pair<StringView, StringView>& p2) noexcept -> bool
    // Sort the completion candidates based on their values and whether they are directories or files.
    //
    // [Args]
    //   p1 (const Pair<StringView, StringView>&): [IN] First pair to compare.
    //   p2 (const Pair<StringView, StringView>&): [IN] Second pair to compare.
    //
    // [Returns]
    //   (bool): True if the first pair should come before the second pair, false otherwise.
    {
        // Get the value of the pairs.
        const StringView s1 = p1.first;
        const StringView s2 = p2.first;

        // Determine if the values are directories (ending with '/').
        const bool is_dir1 = (s1.size() > 0) and (s1.back() == '/');
        const bool is_dir2 = (s2.size() > 0) and (s2.back() == '/');

        // Sort directories before files, and sort lexicographically within each group.
        if      (is_dir1 and is_dir2) return (s1 < s2);
        else if (is_dir1            ) return true;
        else if (            is_dir2) return false;
        else                          return (s1 < s2);
    };

    // Do nothing if no tokens are provided.
    if (tokens.size() < 2) co_return;

    // Prepare the command to run "carapace" with the given tokens.
    uint64_t hash_val = hash(tokens[0]);
    for (SizeType idx = 1; idx < tokens.size(); ++idx)
        hash_val = hash(tokens[idx], hash("\xFF", hash_val));

    if (not this->cache.contains(hash_val))
    {
        // Prepare the command to run "carapace" with the given tokens:
        //     args = ["carapace", tokens[0], "export", tokens[0], tokens[1], ..., ""]
        Vector<StringView> args = {"carapace", tokens.front(), "export"};
        args.insert(args.end(), tokens.begin(), tokens.end() - 1);
        args.emplace_back("");

        try
        {
            // Parse the command output as JSON.
            const json result_json = json::parse(run_command(args, RUN_COMMAND_GETOUT));

            // Initialize flags.
            bool is_dir_completion = false;

            this->cache[hash_val] = {};
            for (const json& item : result_json["values"])
            {
                // Get the completion candidate and its display string from the JSON object.
                const String value       = item.value("value",       "");
                // const String description = item.value("description", "");
                const String display     = item.value("display",     "");
                const String style       = item.value("style",       "");
                const String tag         = item.value("tag",         "");

                // Check if the completion candidate is a directory based on its tag.
                if ((not is_dir_completion) and (tag == "files"))
                    is_dir_completion = true;

                // Register the completion candidate and its display string in the cache.
                this->cache[hash_val].emplace_back(value, display);

                // Colorize the display string based on its style and tag.
                if ((tag == "files") and style.contains("blue"))
                    this->cache[hash_val].back().second = "\x1B[38;2;97;175;239m" + this->cache[hash_val].back().second + "\x1B[m";
            }

            // Sort list results.
            if (is_dir_completion)
                std::sort(this->cache[hash_val].begin(), this->cache[hash_val].end(), sort_func);
        }
        catch (const json::parse_error& e)
        {
            // If an error occurs while parsing the JSON output, return without yielding any completion candidates.
            co_return;
        }
    }

    // Yield the completion candidates that match the last token.
    for (const Pair<String, String>& pair : this->cache[hash_val])
        if (pair.first.starts_with(tokens.back()))
            co_yield {pair.first, pair.second};

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
