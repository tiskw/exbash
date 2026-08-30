////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: tokenizers.cxx                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "tokenizers.hxx"

// Include the headers of custom modules.
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    String replace_path_plugin(StringView token, const Vector<Path>& path_plugin_candidates)
    // Replace "{path_plugin}" in the given token with an appropriate path.
    //
    // [Args]
    //   token                  (StringView)         : [IN] Target token to be replaced.
    //   path_plugin_candidates (const Vector<Path>&): [IN] Candidate paths for "{path_plugin}".
    //
    // [Returns]
    //   (String): Replaced token.
    //
    {   // {{{

        for (const Path& path_plugin : path_plugin_candidates)
        {
            String token_new = replace(token, "{path_plugin}", path_plugin.string());
            if (stdfs::exists(token_new) and stdfs::is_regular_file(token_new))
                return token_new;
        }

        // If no candidate path is valid, return the original token.
        return String(token);

    }   // }}}

    StringViewConstIter get_string_token(StringViewConstIter iter, StringViewConstIter iter_end, const char quote)
    // Get string token that is quoted by single/double quote.
    // This function is used in the "tokenize" function.
    //
    // [Args]
    //   iter     (StringConstIter): [IN]  Start iterator.
    //   iter_end (StringConstIter): [IN]  End iterator.
    //   quote    (const uint64_t) : [IN]  Quote type.
    //
    // [Returns]
    //   (StringConstIter): Iterator pointing to the end of the token.
    //
    {   // {{{

        ++iter;
        while ((iter != iter_end) and (*iter != quote))
            ++iter;
        return (iter != iter_end) ? ++iter : iter;

    }   // }}}

    StringViewConstIter get_wspace_token(StringViewConstIter iter, StringViewConstIter iter_end)
    // Get white-space token.
    // This function is used in the "tokenize" function.
    //
    // [Args]
    //   iter     (StringConstIter): [IN]  Start iterator.
    //   iter_end (StringConstIter): [IN]  End iterator.
    //
    // [Returns]
    //   (StringConstIter): Iterator pointing to the end of the token.
    //
    {   // {{{

        while ((iter != iter_end) and ((*iter == ' ') or (*iter == '\t')))
            ++iter;
        return iter;

    }   // }}}

    StringViewConstIter get_normal_token(StringViewConstIter iter, StringViewConstIter iter_end)
    // Get normal token.
    // This function is used in the "tokenize" function.
    //
    // [Args]
    //   iter     (StringConstIter): [IN]  Start iterator.
    //   iter_end (StringConstIter): [IN]  End iterator.
    //
    // [Returns]
    //   (StringConstIter): Iterator pointing to the end of the token.
    //
    {   // {{{

        while ((iter != iter_end) and (*iter != ' ') and (*iter != '\t'))
            ++iter;
        return iter;

    }   // }}}

    bool is_quoted_token(StringView token)
    // Check if the given token is a quoted token.
    // This function is used in the "tokenize" function.
    //
    // [Args]
    //   token (StringView): [IN] Target token.
    //
    // [Returns]
    //   (bool): True if the token is a quoted token, otherwise false.
    //
    {   // {{{

        // Do not consider the token as a quoted token if its length is less than 2,
        if (token.size() < 2) return false;

        // Check if the token is quoted by single/double quote.
        if (token.starts_with("'")  and token.ends_with("'") ) { return true; }
        if (token.starts_with("\"") and token.ends_with("\"")) { return true; }

        // Otherwise, the token is not a quoted token.
        return false;

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Generator<StringView> tokenize(StringView sv, TokenizeOption option)
{   // {{{

    // Parse the tokenization options.
    const bool keep_whitespaces = (option & TOKENIZE_KEEP_WS) != 0;
    const bool dequote_tokens   = (option & TOKENIZE_DEQUOTE) != 0;

    // Initialize iterators.
    StringViewConstIter iter = sv.cbegin();

    while (iter != sv.cend())
    {
        //
        StringViewConstIter iter_bgn = iter;

        // Read a token.
        switch (*iter_bgn)
        {
            case '\'': iter = get_string_token(iter, sv.cend(), '\''); break;  // String token.
            case '"' : iter = get_string_token(iter, sv.cend(), '"');  break;  // String token.
            case ' ' : iter = get_wspace_token(iter, sv.cend());       break;  // Whitespace token.
            case '\t': iter = get_wspace_token(iter, sv.cend());       break;  // Whitespace token.
            default  : iter = get_normal_token(iter, sv.cend());       break;  // Others.
        }

        // Skip white-space tokens if the option to keep white-spaces is not set.
        if ((not keep_whitespaces) and ((*iter_bgn == ' ') or (*iter_bgn == '\t')))
            continue;

        // Get the token string view.
        StringView token = StringView(iter_bgn, iter - iter_bgn);

        // De-quote the token if the option to de-quote tokens is set.
        if (dequote_tokens and is_quoted_token(token))
            token = token.substr(1, token.size() - 2);

        co_yield token;
    }

}   // }}}

Generator<String> tokenize_with_placeholder_replacement(StringView command, const StringMap& extra, TokenizeOption option)
{   // {{{

    // Get home directory path from the environment variable.
    const char* home = std::getenv("HOME");

    // Define candidate paths for the "{path_plugin}".
    const Vector<Path> path_plugin_candidates = {
        ((home != nullptr) ? Path(home) : Path(".")) / ".config/exbash/plugins",
        ((home != nullptr) ? Path(home) : Path(".")) / ".local/share/exbash/plugins",
    };

    // Tokenize the command string.
    for (const StringView token_view : tokenize(command, option))
    {
        String token(token_view);

        // Replace "{path_plugin}" with an appropriate path.
        if (token.starts_with("{path_plugin}"))
            token = replace_path_plugin(token, path_plugin_candidates);

        // Replace the token with the value in the "extra" map.
        else if (extra.contains(token))
            token = extra.at(token);

        // Replace tilde to home directory path.
        if (token.starts_with("~") and (home != nullptr))
            token = String(home) + String(token.substr(1));

        co_yield token;
    }

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
