////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: edit_helper.cxx                                                             ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "edit_helper.hxx"

// Include STL headers.
#include <algorithm>
#include <regex>

// Include the headers of custom modules.
#include "cmd_runner.hxx"
#include "path_x.hxx"
#include "preview.hxx"
#include "string_utils.hxx"
#include "tokenizers.hxx"
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    Vector<String> columnize(const Vector<StringView>& texts, Size area_size, uint16_t padding)
    // Columnize the given string vector.
    //
    // [Args]
    //   texts     (const Vector<StringView>&): [IN] Input string vector.
    //   area_size (Size)                     : [IN] Maximum size of display (columns and rows).
    //   padding   (uint16_t)                 : [IN] Padding width between columns.
    //
    // [Returns]
    //   (String): Columnized string.
    //
    {   // {{{

        constexpr auto get_shape = [](const Vector<uint16_t>& ws, uint16_t width, uint16_t margin, uint16_t n_rows) noexcept -> Tuple<uint16_t, bool>
        // Compute shape of column style display.
        //
        // [Args]
        //   ws     (const Vector<uint16_t>&): [IN] Length of each text.
        //   width  (uint16_t)               : [IN] Maximum width of display.
        //   margin (uint16_t)               : [IN] Minimum margin between each text.
        //   n_rows (uint16_t)               : [IN] Number of rows of the display.
        //
        // [Returns]
        //   (Tuple<uint16_t, bool>): A pair of (number of columns, true if all texts can be shown).
        {
            uint16_t wid_total = 0;

            // Create columns and append them to each line.
            for (uint16_t col = 0; true; ++col)
            {
                // Compute start/end index of the texts used in the current column
                const size_t idx_bgn = col * n_rows;
                const size_t idx_end = min(idx_bgn + n_rows, ws.size());

                // Compute maximum width of texts used in the current column.
                const uint16_t wid_max = (idx_end > idx_bgn) ? *std::max_element(ws.begin() + idx_bgn, ws.begin() + idx_end) : 0;

                // Increment of width by this column.
                const uint16_t wid_inc = ((col > 0) ? margin : 0) + wid_max;

                // Exit if the current width exceeds the maximum width.
                if ((wid_total + wid_inc) >= width)
                    return {max(static_cast<uint16_t>(1), col), false};

                // Exit if all text was used.
                if (idx_end == ws.size())
                    return {col + 1, true};

                // Update current width.
                wid_total += wid_inc;
            }
        };

        constexpr auto get_optimal_shape = [get_shape](const Vector<uint16_t>& ws, Size area_size, uint16_t margin) noexcept -> Size
        // Compute optimal shape (rows and columns) of column style display.
        //
        // [Args]
        //   ws  (const Vector<uint16_t>): [IN] Input texts.
        //   width  (uint16_t)           : [IN] Maximum width of display.
        //   height (uint16_t)           : [IN] Maximum height of display.
        //   margin (uint16_t)           : [IN] Minimum margin between each text.
        //
        // [Returns]
        //   (Size): A pair of rows and columns of the optimal shape.
        {
            for (uint16_t row = 1; row < area_size.rows; ++row)
            {
                // Compute shape for each row.
                const auto [col, finished] = get_shape(ws, area_size.cols, margin, row);

                // Immediately determine optimal shape if all texts can be shown.
                if (finished)
                    return {col, row};
            }

            // Compute column if row is equal with the maximum height.
            const auto [col, _] = get_shape(ws, area_size.cols, margin, area_size.rows);

            return {col, area_size.rows};
        };

        // Prepare output lines.
        Vector<String> lines;
        for (uint16_t row = 0; row < area_size.rows; ++row)
            lines.emplace_back("");

        // Do nothing if no text is given.
        if (texts.size() == 0)
            return lines;

        // Get width of each text.
        Vector<uint16_t> ws = transform<StringView, uint16_t>(texts, width);

        // Compute optimal shape (rows and columns).
        const Size column_shape = get_optimal_shape(ws, area_size, padding);

        // Initialize total width.
        uint16_t width_total = 0;

        // Create columns and append them to each line.
        for (uint16_t col = 0; col < column_shape.cols; ++col)
        {
            // Compute start/end index of the texts used in the current column
            const size_t idx_bgn = col * column_shape.rows;
            const size_t idx_end = min(idx_bgn + column_shape.rows, ws.size());

            // Compute maximum width of texts used in the current column.
            const uint16_t wid_max = (idx_end > idx_bgn) ? *std::max_element(ws.cbegin() + idx_bgn, ws.cbegin() + idx_end) : 0;

            // Append texts to each line.
            for (uint16_t idx = idx_bgn; idx < idx_end; ++idx)
            {
                lines[idx % column_shape.rows] += texts[idx];

                // Append margin whitespaces.
                if (col < (column_shape.cols - 1))
                    lines[idx % column_shape.rows] += String(wid_max + padding - ws[idx], ' ');
            }

            // Update total width.
            width_total += wid_max + padding;

            // Exit function if the current width exceeds the given width.
            if (width_total > area_size.cols) break;
        }

        return lines;

    }   // }}}

    Vector<Vector<RegEx>> compile_regex_patterns(const Vector<Completion>& completions)
    // Compile the regular expression patterns in the config file and store them in the given vector.
    //
    // [Args]
    //   completions (const auto&): [IN] Vector of completion targets in the config file.
    //
    // [Returns]
    //   (Vector<Vector<std::regex>>): Vector of compiled regular expression patterns.
    //
    {   // {{{

        // Initialize the output vector.
        Vector<Vector<RegEx>> vec_regex;

        // Compile the regular expression patterns in the config file.
        for (const auto& [patterns, comp_type, option] : completions)
        {
            Vector<std::regex> patterns_regex;
            for (const String& pattern : patterns)
                patterns_regex.emplace_back(RegEx(pattern));

            vec_regex.push_back(std::move(patterns_regex));
        }

        return vec_regex;

    }   // }}}

    Vector<String> get_available_commands(const String& path_cmnd_info, const String& path_bash_info)
    // Get a list of all available commands in Bash.
    //
    // [Args]
    //   path_cmnd_info (const String&): [IN] Path to ExBash's command info file.
    //   path_bash_info (const String&): [IN] Path to ExBash's bash info file.
    //
    // [Returns]
    //   (Vector<String>): List of available command names.
    //
    {   // {{{

        // Initialize the output vector.
        Vector<String> result;

        // Append the Bash bash info.
        for (const String& line : readline(path_bash_info.c_str()))
            if (!line.empty() and isalpha(line[0]))
                result.emplace_back(strip(line));

        // Append the path commands info.
        for (const String& line : readline(path_cmnd_info.c_str()))
            if (!line.empty() and isalpha(line[0]))
                result.emplace_back(strip(line));

        // Sort and remove duplicated command names.
        deduplicate(result);

        return result;

    }   // }}}

    Generator<String> get_options_from_help(const String& command)
    // Get help message of the specified command, parse it, and get option info.
    //
    // [Args]
    //   command (const String&): [IN] Command string.
    //
    // [Returns]
    //   (Generator<String>): List of short and long options.
    //
    // [Notes]
    //   The data type of the argument "command" should be "const String" instead of "const String&",
    //   because this function is a generator and its lifetime may be longer than the usual function.
    //   If the argument is "const String&", the reference can be invalid when the last "co_yield" is executed.
    //
    {   // {{{

        // Initialize the output set.
        OrderedSet<String> output_s;
        OrderedSet<String> output_l;

        // Get the help message of the target command.
        const String help = run_command(std::format("timeout 0.1s {} --help", command), RUN_COMMAND_GETOUT);

        // Define a pattern to detect options.
        const std::regex re(R"((--[\w-]+|-\w)(\[=[\w-]+\])?(=[\w-]+)?[^\w-])");

        for (std::sregex_iterator it(help.begin(), help.end(), re), end; it != end; ++it)
        {
            // Get the option string.
            const String opt = it->str(1) + it->str(2) + it->str(3);

            // Store the option to the set.
            if (opt.starts_with("--")) { output_l.insert(opt); }
            else                       { output_s.insert(opt); }
        }

        // Yields the options in the order of short and long options.
        for (const String& opt : output_s) { co_yield opt; }
        for (const String& opt : output_l) { co_yield opt; }

    }   // }}}

    Tuple<bool, SizeType> match(const Vector<String>& patterns, const Vector<StringView>& tokens, const Vector<std::regex>& patterns_regex)
    // Return True if the given tokens matched with the given patterns.
    // The arguments `tokens` is a list of strings, and the argument `patterns`
    // are list of regular expression strings with the following extra special commands:
    //
    //   * "FILE": existing file path
    //   * ">>"  : skip tokens (available only 1 time in one patterns)
    //
    // [Args]
    //   patterns (const Vector<String>&): [IN] List of regular expression strings.
    //   tokens   (const Vector<String>&): [IN] List of strings to be matched.
    //
    // [Returns]
    //   (bool)    : True is the given tokens matched with the given patterns.
    //   (SizeType): Hash value of the matched tokens.
    //
    {   // {{{

        // Initialize token index.
        SizeType index_token = 0;

        // Initialize
        uint64_t hash_val = hash(nullptr);

        // Run the for-loop based on the pattern index.
        for (SizeType index_pattern = 0; index_pattern < patterns.size(); ++index_pattern)
        {
            // If the number of patterns is longer than the number of tokens
            // (i.e. token finished but pattern is exists yet), then returns false.
            if (index_token >= tokens.size())
                return {false, 0};

            // Select target pattern and token.
            const String&    pattern = patterns[index_pattern];
            const StringView token   = tokens[index_token];

            // Select the compiled regular expression pattern.
            const std::regex& pattern_regex = patterns_regex[index_pattern];

            // Case 1: pattern is ">>".
            if (pattern == ">>")
                index_token = tokens.size() - patterns.size() + index_pattern;

            // Case 2: pattern is "FILE" but the file not exists.
            else if (pattern == "FILE" and (not stdfs::exists(token)))
                return {false, 0};

            // Case 3: pattern is "FILE" and the file exists.
            else if (pattern == "FILE")
                /* pass */;

            // Case 4: others.
            else if (not regex_match(token.begin(), token.end(), pattern_regex))
                return {false, 0};

            // Update the hash value if the pattern is not ">>" that means skip.
            // Note: In order to avoid hash collision by an empty token, the hash value of
            //       an empty token is computed as the hash value of a newline character "\n".
            if (pattern != ">>")
                hash_val = hash(token.empty() ? "\n" : token, hash_val);

            ++index_token;
        };

        // No unprocessed tokens remains if the token matches with the pattern.
        return {index_token == tokens.size(), hash_val};

    }   // }}}

    Tuple<CompType, String, SizeType> get_target(const Vector<StringView>& tokens, const Vector<Completion>& completions, const Vector<Vector<std::regex>>& vec_patterns_regex)
    // Returns a pair of target completion type and its optional string.
    //
    // [Args]
    //   tokens (const Vector<String>&): [IN] List of strings to be matched.
    //
    // [Returns]
    //   (CompType): Target completion type.
    //   (String)  : Optional string of the target completion.
    //
    {   // {{{

        for (SizeType idx = 0; idx < completions.size(); ++idx)
        {
            // Get the completion target.
            const auto& [patterns, comp_type, option] = completions[idx];

            // Get the compiled regular expression patterns.
            const Vector<std::regex>& patterns_regex = vec_patterns_regex[idx];

            // Check if the given tokens match with the current pattern.
            const auto [is_matched, hash_val] = match(patterns, tokens, patterns_regex);

            // Returns the current completion type and its optional string.
            if (is_matched)
                return {comp_type, option, hash_val};
        }

        return {CompType::NONE, String(""), SizeType(0)};

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EditHelper: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

EditHelper::EditHelper(uint16_t rows, uint16_t cols, const ExBashConfig& cfg) : area_size(cols, rows), column_padding(cfg.column_padding),
    completions(cfg.completions), previews(cfg.previews), preview_delim(cfg.preview_delim), preview_ratio(cfg.preview_ratio)
{   // {{{

    // Initialize shared future instances for caching command names and compiled regular expression patterns.
    std::call_once(this->flag_init_shared_futures, &EditHelper::init_shared_futures, cfg);

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EditHelper: Member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<String> EditHelper::candidate(StringView lhs)
{   // {{{

    // Split the given text (left hand side of the cursor) to tokens.
    // Drop white-space tokens and convert String to String.
    Vector<StringView> tokens;
    for (const StringView token : tokenize(lhs, TOKENIZE_PLAIN))
        tokens.emplace_back(token);

    // If the completion key (= lhs) exists in the cache of lhs, use it.
    uint64_t hash_lhs = hash(lhs);
    if (this->cache_cands_lhs.contains(hash_lhs))
        return this->candidate_from_cache(this->cache_cands_lhs, hash_lhs);

    // Add empty token if the editing line ends with white-space.
    if (lhs.size() > 0 and lhs.back() == ' ')
        tokens.push_back(StringView(""));

    // Get completion type and its optional string.
    const auto& [comp_type, option, hash_mat] = get_target(tokens, this->completions, this->shared_future_vec_patterns_regex.get());

    // If the completion key (= matched token) exists in the cache of matched token, use it.
    // NOTE: BASHCOMP is excluded from mat-cache because its pattern [">>", ".*"] skips all leading
    //       tokens (including the command name) when computing hash_mat, so two inputs with the
    //       same partial word but different commands (e.g. "pip insta" vs "cargo insta") would
    //       incorrectly share a single cache entry.  The lhs-cache (keyed on the full input) is
    //       sufficient for BASHCOMP.
    // if ((comp_type != CompType::BASHCOMP) and (comp_type != CompType::SC_AND_BC) and this->cache_cands_mat.contains(hash_mat))
    //     return this->candidate_from_cache(this->cache_cands_mat, hash_mat);

    // Clear completion candidates before computed by the following switch statement.
    this->cands.clear();

    // Compute lines of completion candidates that will be displayed to users.
    switch (comp_type)
    {
        case CompType::BASHCOMP : this->cands_bashcomp (lhs, tokens);         break;
        case CompType::CARAPACE : this->cands_carapace (tokens);              break;
        case CompType::COMMAND  : this->cands_command  (tokens, option);      break;
        case CompType::GREP     : this->cands_grep     (tokens, option);      break;
        case CompType::OPTION   : this->cands_option   (tokens);              break;
        case CompType::PATH     : this->cands_filepath (tokens);              break;
        case CompType::PREVIEW  : this->cands_filepath (tokens);              break;
        case CompType::SHELL    : this->cands_shell    (tokens, option);      break;
        case CompType::SUBCMD   : this->cands_subcmd   (tokens, option);      break;
        case CompType::SC_AND_BC: this->cands_sc_and_bc(lhs, tokens, option); break;
        case CompType::NONE     : this->cands_filepath (tokens);              break;
    }

    // Convert completion candidates to lines for display.
    this->lines_from_cands(this->cands);

    // Update completion lines using preview info.
    if (comp_type == CompType::PREVIEW)
        this->cands_preview(tokens);

    // Register the completion result to the cache.
    if (comp_type != CompType::NONE)
    {
        this->cache_cands_lhs[hash_lhs] = {this->cands, this->lines};
        // BASHCOMP is excluded from mat-cache for the same reason explained above.
        if (comp_type != CompType::BASHCOMP)
            this->cache_cands_mat[hash_mat] = {this->cands, this->lines};
    }

    return this->lines;

}   // }}}

String EditHelper::complete(StringView lhs) const
{   // {{{

    constexpr auto get_common_substr = [](const Vector<String>& texts) noexcept -> StringView
    // Get common substring of the given strings.
    //
    // [Args]
    //   texts (const Vector<String>&): [IN] List of strings.
    //
    // [Returns]
    //   (String): Common string in the given list of strings.
    {
        // Returns empty string if the size of the given text list is zero.
        if (texts.size() == 0) return StringView("");

        // Returns entire string if the size of the given text list is one.
        if (texts.size() == 1) return StringView(texts[0]);

        // Find minimum length of the given texts.
        SizeType min_size = texts[0].size();
        for (SizeType n = 1; n < texts.size(); ++n)
            min_size = min(min_size, texts[n].size());

        // Check consistency for each character and append to the result string.
        for (SizeType m = 0; m < min_size; ++m)
        {
            // Check the character consistency.
            for (SizeType n = 1; n < texts.size(); ++n)
                if (texts[n][m] != texts[0][m])
                    return StringView(texts[0].c_str(), m);
        }

        return StringView(texts[0].c_str(), min_size);
    };

    // Split the given text (left hand side of the cursor) to tokens.
    Vector<StringView> tokens;
    for (const StringView token : tokenize(lhs, TOKENIZE_KEEP_WS))
        tokens.emplace_back(token);

    // Do nothing if token is empty.
    if (tokens.size() == 0) return String(lhs);

    // Get number of completion candidates.
    const size_t num_cands = this->cands.size();

    // Do nothing if no candidate given.
    if (num_cands == 0) return String(lhs);

    // Concatenate tokens except the last token.
    // If the last token is a whitespace token, then do not drop the last token.
    StringView lhs_without_last_token = (
        tokens.back().starts_with(" ") or tokens.back().starts_with("\t") or not this->cands[0].first.starts_with(tokens.back())
    ) ? StringView(lhs) : StringView(lhs.data(), lhs.size() - tokens.back().size());

    // Compute completion string.
    if ((num_cands == 1) and this->cands[0].first.ends_with('/'))
    {
        // Extra slash will be added when directory path is completed.
        // However, slash is already added to the completion token,
        // therefore just adding the completion token is enough.
        return String(lhs_without_last_token) + this->cands[0].first;
    }
    else if (num_cands == 1)
    {
        // Add extra white-space at the end if number of completion candidate is one.
        return String(lhs_without_last_token) + this->cands[0].first + ' ';
    }
    else
    {
        // Create an array of completion strings.
        constexpr auto get_first = [](const Pair<String, String>& pair) noexcept -> String { return pair.first; };
        Vector<String> keys = transform<Pair<String, String>, String>(this->cands, get_first);

        return String(lhs_without_last_token) + String(get_common_substr(keys));
    }

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EditHelper: Private functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<String> EditHelper::candidate_from_cache(const CandCacheMap& cache, uint64_t hash_key)
{   // {{{

    // Get the cached candidates and lines.
    const auto& [cands_cache, lines_cache] = cache.at(hash_key);

    // Repace the current candidates/lines with the cached candidates/lines.
    this->cands = cands_cache;
    this->lines = lines_cache;

    return this->lines;

}   // }}}

void EditHelper::cands_bashcomp(StringView lhs, const Vector<StringView>& tokens)
{   // {{{

    for (const String& c : this->bash_completer.complete(lhs))
        this->cands.emplace_back(c, c);

    if (this->cands.empty())
        this->cands_filepath(tokens);

}   // }}}

void EditHelper::cands_command(const Vector<StringView>& tokens, const String& option)
{   // {{{

    constexpr auto description = [](StringView cmd, StringView token) -> String
    // Returns colorized command name for description of the compretion.
    //
    // [Args]
    //   cmd   (const String&): [IN] Command name to be displayed.
    //   token (const String&): [IN] User input token for the command name.
    //
    // [Returns]
    //   (String): Colorized command name for display.
    {
        if (token.empty() or cmd.size() < token.size())
            return String(cmd);

        return std::format("\x1B[35m{}\x1B[0m{}", token, cmd.substr(token.size()));
    };

    // The `option` should be empty string.
    if (option.size() > 0) return;

    // Get query token.
    const StringView token = tokens[0];

    // Prepare command cache.
    if (this->cache_commands.size() == 0)
        for (const String& cmd : this->shared_future_cache_commands.get())
            this->cache_commands.emplace_back(cmd);

    // Filter matched command names.
    for (const String& cmd : this->cache_commands)
        if (cmd.starts_with(token))
            this->cands.emplace_back(String(cmd), description(cmd, token));

}   // }}}

void EditHelper::cands_carapace(const Vector<StringView>& tokens)
{   // {{{

    for (const Pair<StringView, StringView>& pair : this->carapace_service.complete(tokens))
        this->cands.emplace_back(pair.first, pair.second);

}   // }}}

void EditHelper::cands_filepath(const Vector<StringView>& tokens)
{   // {{{

    constexpr auto colorize_name = [](const String& name, const Path& path, const String& query_key) noexcept -> String
    // Colorize the file name based on the file type and the user input query key.
    //
    // [Args]
    //   name      (const String&): [IN] File name to be colorized.
    //   path      (const Path&  ): [IN] File path for checking the file type.
    //   query_key (const String&): [IN] User input query key for colorization.
    //
    // [Returns]
    //   (String): Colorized file name for display.
    {
        const char* color_code = "\x1B[0m";

        // Case 1: Directory.
        if ((name.size() > 0) and (name.back() == '/'))
            color_code = "\x1B[94m";

        // Case 2: executable file.
        else if ((stdfs::status(path).permissions() & stdfs::perms::owner_exec) != stdfs::perms::none)
             color_code = "\x1B[92m";

        // Returns withour query colorization if the query key is empty or the query key is invalid.
        if (query_key.empty() or name.size() < query_key.size())
            return color_code + name + "\x1B[0m";

        // Colorize the matched query key.
        String result = "\x1B[35m" + name + "\x1B[0m";
        result.insert(query_key.size() + 5, color_code);

        return result;
    };

    // Split user input token to a tuple of:
    //   * directory path to be searched,
    //   * query string for filtering seach result.
    const auto [query_dir, query_key] = split_to_target_and_query(tokens);

    // Show dot file if current file name started with dot.
    const bool show_dot = (query_key.size() > 0) and (query_key[0] == '.');

    // Search the directory and filter out unnecessary search results.
    for (const String& name : query_dir.listdir())
    {
        // Skip dot files if the query key is not a dot file.
        if ((not show_dot) and (name[0] == '.'))
            continue;

        // If match with the user input.
        if (name.starts_with(query_key))
        {
            // Compute path of the target file.
            Path path = query_dir / name;

            // Get the colorised name as a description of the completion.
            String desc = colorize_name(name, path, query_key);

            // Append query string and display string.
            this->cands.emplace_back(String(path.c_str()), String(desc));
        }
    }

}   // }}}

void EditHelper::cands_grep(const Vector<StringView>& tokens, const String& option)
{   // {{{

    // Get query token.
    const StringView token = (tokens.size() > 0) ? tokens.back() : StringView("");

    // Find the position of the first tab character in the option string.
    // If the tab character is not found, then the option string is invalid.
    const SizeType sep = option.find('\t');
    if (sep == String::npos) return;

    // Get the target file path.
    const String path = expand_tilde(option.substr(0, sep));

    // Get the regular expression pattern.
    const RegEx pattern(option.substr(sep + 1));

    // Create a matching result instance.
    std::smatch match;

    // Read lines from the file and print the match result if the pattern is matched.
    for (const String& line : readline(path.c_str()))
    {
        // Skip if not matched with the pattern.
        if (not std::regex_search(line, match, pattern))
            continue;

        // Get the target string that will be displayed as a completion candidate.
        String target = (match.size() > 1) ? match[1] : line;

        if (target.starts_with(token))
            this->cands.emplace_back(target, target);
    }

}   // }}}

void EditHelper::cands_option(const Vector<StringView>& tokens)
{   // {{{

    constexpr auto colorize_description = [](StringView desc, StringView token) -> String
    // Colorize the command name in the description based on the user input token.
    //
    // [Args]
    //   desc  (const String&): [IN] Description string to be colorized.
    //   token (const String&): [IN] User input token for colorization.
    //
    // [Returns]
    //   (String): Colorized description string for display.
    {
        // Returns without colorization if the user input token is empty or the token is invalid.
        if (token.empty() or desc.size() < token.size())
            return String(desc);

        // Get the color code for the command name in the description.
        StringView color = StringView("");
        if (desc.starts_with("\x1B["))
            color = StringView(desc.data(), desc.find('m') + 1);

        return std::format("\x1B[35m{}{}{}", token, color, desc.substr(token.size() + color.size()));
    };

    // Regular expression patterns for colorization.
    static const std::regex pattern_color1(R"(^--[\w-]+|^-\w)");
    static const std::regex pattern_color2(R"((=)([\w-]+))");

    // Get the target command.
    const String command = String((tokens.size() > 0) ? tokens[0] : StringView(""));

    // Run command with "--help" option if not registered in the cache.
    if (not this->opt_cache.contains(command))
    {
        // Create new map instance.
        this->opt_cache[command] = Vector<Tuple<String, String>>();

        for (const String& opt : get_options_from_help(command))
        {
            // Get colorized description.
            String desc = opt;
            desc = std::regex_replace(desc, pattern_color1,   "\x1B[94m$&\x1B[0m");
            desc = std::regex_replace(desc, pattern_color2, "$1\x1B[93m$2\x1B[0m");

            // Append to the cache vector.
            this->opt_cache[command].emplace_back(opt, desc);
        }
    }

    // Get query token.
    const StringView token = (tokens.size() > 0) ? tokens.back() : StringView("");

    // Add matched options.
    for (const auto& [opt, desc] : this->opt_cache[command])
    {
        // Skip if not matched with the current token.
        if (not opt.starts_with(token))
            continue;

        // Append to the candidate list.
        this->cands.emplace_back(opt, colorize_description(desc, token));
    }

}   // }}}

void EditHelper::cands_preview(const Vector<StringView>& tokens)
{   // {{{

    constexpr auto get_last_nonwhitespace_token = [](const Vector<StringView>& tokens) noexcept -> StringView
    // Returns last non-whitespace token.
    //
    // [Args]
    //   tokens (Vector<StringView>&): [IN] Target tokens.
    //
    // [Returns]
    //   (String): Non-whitespace token.
    {
        // Search tokens from the tail.
        for (PtrDiff idx = tokens.size() - 1; idx >= 0; --idx)
            if (tokens[idx].size() > 0 and tokens[idx][0] != ' ')
                return tokens[idx];

        // Returns brank string if not found.
        return StringView("");
    };

    // Get the target file path that is a last non-white-space token.
    const StringView path = get_last_nonwhitespace_token(tokens);

    // Compute width of the preview window.
    const uint16_t width_prev = this->area_size.cols - int(this->area_size.cols * this->preview_ratio) - this->preview_delim.size();

    // Get preview result.
    Vector<String> preview_lines = preview(path, this->area_size.rows, this->previews);

    // Append preview lines to the current completion lines.
    for (size_t idx = 0; (idx < preview_lines.size()) and (idx < (size_t) this->area_size.rows); ++idx)
    {
        // Get the width of the current line.
        const uint16_t width_line = width(this->lines[idx]);

        // Clip the line to the half-width `w`.
        if (width_line > width_prev)
            this->lines[idx] = String(textclip(this->lines[idx], width_prev));

        // Add extra white-space if the line is shorter than the half-width `w`.
        else if (width_line < width_prev)
            this->lines[idx] += String(width_prev - width_line, ' ');

        // Append preview delimiter and line to the target line.
        this->lines[idx] += String("\033[m") + preview_delim + preview_lines[idx];

        // Clip the target line again.
        this->lines[idx] = String(textclip(this->lines[idx], this->area_size.cols - 1));
    }

}   // }}}

void EditHelper::cands_shell(const Vector<StringView>& tokens, const String& option)
{   // {{{

    // Get the target token.
    const StringView token = (tokens.size() > 0) ? tokens.back() : StringView("");

    // Tokenize the given command option and replace placeholder if exists.
    Vector<String> cmd_tokens;
    for (const StringView cmd_token : tokenize_with_placeholder_replacement(option, {}, TOKENIZE_DEQUOTE))
        cmd_tokens.emplace_back(cmd_token);

    // Run specified command.
    const String output = run_command(cmd_tokens, RUN_COMMAND_GETOUT);

    for (StringView line : split(output, "\n"))
    {
        // Strip line.
        line = strip(line);

        // Get the position of the first whitespace in the line.
        SizeType pos_ws = line.find(' ');

        // Get 1st token of each line.
        const StringView line_1st_token = (pos_ws != String::npos) ? StringView(line.data(), pos_ws) : StringView(line);

        // Register matched output lines.
        if (line_1st_token.starts_with(token))
            this->cands.emplace_back(line_1st_token, line);
    }

}   // }}}

void EditHelper::cands_subcmd(const Vector<StringView>& tokens, const String& option)
{   // {{{

    // Get the target token.
    const StringView token = (tokens.size() > 0) ? tokens.back() : String("");

    // Run the given command and add the result to the cache
    // if the given command is not registered in the cache.
    if (not this->subcmd_cache.contains(option))
    {
        // Run command and register each line.
        for (const StringView line : split(run_command(option, RUN_COMMAND_GETOUT), "\n"))
            if (line.starts_with("  ") and not line.starts_with("     "))
                this->subcmd_cache[option].emplace_back(strip(line));
    }

    for (const String& line : this->subcmd_cache[option])
    {
        // Skip unmatched line.
        if (not line.starts_with(token))
            continue;

        // Get the position of the first whitespace in the line.
        SizeType pos_ws = line.find(' ');

        // If the line doesn't have a space, the entire line is a completion candidate and description.
        if (pos_ws == String::npos)
        {
            this->cands.emplace_back(line, line);
            continue;
        }

        // Split the line into a command name and an explanation.
        StringView name = StringView(line.data(), pos_ws);
        StringView expl = StringView(line.data() + name.size(), line.size() - name.size());

        // Initialize the description string.
        String desc;

        // Append the command name to the description string with colorization.
        if (token.empty() or (name.size() < token.size()))
            desc = std::format("\x1B[32m{}\x1B[m", name);
        else
            desc = std::format("\x1B[35m{}\x1B[32m{}\x1B[0m", token, name.substr(token.size()));

        // Memorize the start position of the explanation.
        SizeType pos_start_expl = desc.size();

        // Append explanation to the description string.
        desc += expl;

        // Replace the seperator whitespace to dot for better visibility.
        SizeType idx = pos_start_expl;
        while ((++idx < desc.size()) and (desc[idx] == ' '))
            desc[idx] = '.';
        if (idx > pos_start_expl)
            desc[--idx] = ' ';

        this->cands.emplace_back(name, desc);
    }

}   // }}}

void EditHelper::cands_sc_and_bc(StringView lhs, const Vector<StringView>& tokens, const String& option)
{   // {{{

    // Case 1: tokens == ["command name"].
    if (tokens.size() <= 1)
        ; /* Do nothing. */

    // Case 2: tokens == ["command name", "something"].
    else if (tokens.size() == 2)
        this->cands_subcmd(tokens, option);

    // Case 3: tokens == ["command name", "subcommand", "something"].
    else
        this->cands_bashcomp(lhs, tokens);

}   // }}}

void EditHelper::lines_from_cands(const Vector<Pair<String, String>>& cands)
{   // {{{

    constexpr auto get_second_view = [](const Pair<String, String>& pair) noexcept -> StringView
    // Returns the second element of the given pair as StringView.
    //
    // [Args]
    //   pair (const Pair<String, String>&): [IN] Target pair.
    //
    // [Returns]
    //   (StringView): The second element of the given pair as StringView.
    {
        return StringView(pair.second);
    };

    // Get descriptions of the completion candidates.
    Vector<StringView> texts = transform<Pair<String, String>, StringView>(cands, get_second_view);

    // Format descriptions in a column style.
    this->lines.clear();
    for (const String& line : columnize(texts, this->area_size, this->column_padding))
        this->lines.push_back(line);

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EditHelper: Private static member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void EditHelper::init_shared_futures(const ExBashConfig& cfg)
{   // {{{

    // Create the cache of available command names.
    shared_future_cache_commands = launch_async(get_available_commands, cfg.path_cmnd_info, cfg.path_bash_info).share();

    // Create the cache of compiled regular expression patterns for completion matching.
    shared_future_vec_patterns_regex = launch_async(compile_regex_patterns, cfg.completions).share();

};  // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EditHelper: Static member variables
////////////////////////////////////////////////////////////////////////////////////////////////////

std::shared_future<Vector<String>> EditHelper::shared_future_cache_commands;
// Shared future instance of the command cache.

std::shared_future<Vector<Vector<RegEx>>> EditHelper::shared_future_vec_patterns_regex;
// Cache of compiled regular expression patterns for completion matching.

std::once_flag EditHelper::flag_init_shared_futures;
// A flag for one-time initialization of shared future instances.

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
