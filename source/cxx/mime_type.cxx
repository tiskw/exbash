////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: mime_type.cxx                                                               ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "mime_type.hxx"

// Include STL headers.
#include <fnmatch.h>
#include <fstream>

////////////////////////////////////////////////////////////////////////////////////////////////////
// MimeType: Constructors
////////////////////////////////////////////////////////////////////////////////////////////////////

MimeType::MimeType(void)
{   // {{{

    // Open the file.
    std::ifstream ifs("/usr/share/mime/globs");
    if (not ifs) return;

    // Temporal string for reading the file.
    String line;

    while (std::getline(ifs, line))
    {
        // Skip if commented out.
        if ((line.size() == 0) || (line[0] == '#') || (line.find(':') == String::npos))
            continue;

        // Find the location of colon because each line of the file is a "colon-separated" value.
        const SizeType index_colon = line.find_first_of(':');

        // Parse line to mime type and pattern string.
        const String mime_type = line.substr(0, index_colon);
        const String pattern   = line.substr(index_colon + 1, String::npos);

        // Register if the pattern is "*.xxx".
        if (pattern.starts_with("*."))
            this->mime_database[pattern.substr(1)] = mime_type;
    }

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// MimeType: Member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

String MimeType::get(StringView path) const
{   // {{{

    // Do nothing if the path is not a file.
    if (stdfs::is_directory(path))
        return "inode/directory";

    // Get file name as a preprocessing for pattern matching.
    const String suffix = Path(path).extension();

    //
    return (this->mime_database.contains(suffix)) ? this->mime_database.at(suffix) : "text/plain";

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
