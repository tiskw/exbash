////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: test_file_model.cxx                                                         ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the headers of STL.
#include <algorithm>
#include <clocale>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Include POSIX headers.
#include <unistd.h>

// Include the test header.
#include "test_common.hxx"

// Include the headers of custom modules.
#include "config.hxx"
#include "dtypes.hxx"
#include "file_model.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions used for the test
////////////////////////////////////////////////////////////////////////////////////////////////////

// Link-time stub.
// The model cache path is tested without pulling in preview.cxx/config.cxx.
Vector<String> preview(const Path& path, int height)
{   // {{{

    return Vector<String>{"stub-preview", path.filename().string(), std::to_string(height)};

}   // }}}

namespace
{
    struct TempDir
    {   // {{{

        stdfs::path path;

        TempDir()
        {
            path = stdfs::temp_directory_path() / ("filechooser_test_" + std::to_string(::getpid()) + "_model");
            stdfs::remove_all(path);
            stdfs::create_directories(path);
        }
        ~TempDir()
        {
            std::error_code ec;
            stdfs::remove_all(path, ec);
        }

    };  // }}}

    bool ends_with(const String& s, const String& suffix)
    {   // {{{

        return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;

    }   // }}}

    std::vector<String> names_of(const Vector<ListItem>& items)
    {   // {{{

        std::vector<String> names;
        for (const auto& item : items)
            names.push_back(item.left);
        return names;

    }   // }}}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test function
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void)
{   // {{

    std::setlocale(LC_ALL, "");

    FilechooserConfig cfg;

    TempDir td;
    stdfs::create_directory(td.path / "adir");
    stdfs::create_directory(td.path / "zdir");
    std::ofstream(td.path / "b.txt") << "b";
    std::ofstream(td.path / "c.txt") << "c";
    std::ofstream(td.path / ".hidden") << "h";
    std::ofstream(td.path / "adir" / "inside.txt") << "inside";

    FilerModel model(td.path, cfg);
    auto names = names_of(model.get_contents_filt());
    assert(names[0] == "adir");
    assert(names[1] == "zdir");
    assert(std::find(names.begin(), names.end(), ".hidden") == names.end());
    assert(model.get_focus_filt() == 0);
    assert(model.get_item().left == "adir");

    model.process_key("j", "");
    assert(model.get_focus_filt() == 1);
    assert(model.get_item().left == "zdir");

    model.process_key("^F", "");
    assert(model.get_focus_filt() == 3);

    model.process_key("^B", "");
    assert(model.get_focus_filt() == 0);

    model.process_key("0", "");
    assert(model.get_focus_filt() == 0);

    model.process_key("G", "");
    assert(model.get_item().left == "c.txt");
    model.process_key("k", "");
    assert(model.get_item().left == "b.txt");

    model.process_key(" ", "");
    auto selected = model.get_selected();
    assert(selected.size() == static_cast<size_t>(1));
    assert(ends_with(selected[0], "b.txt"));

    assert(model.get_contents_prev().size() > 0);
    assert(model.get_focus_prev() == 0);

    // A fresh model has no starred item, so get_selected() falls back to the focused item.
    FilerModel model2(td.path, cfg);
    assert(model2.get_item().left == "adir");
    selected = model2.get_selected();
    assert(selected.size() == static_cast<size_t>(1));
    assert(ends_with(selected[0], "adir"));

    model2.process_key("", "c");
    assert(model2.get_contents_filt().size() == static_cast<size_t>(1));
    assert(model2.get_item().left == "c.txt");

    model2.process_key(".", "");
    names = names_of(model2.get_contents_filt());
    assert(std::find(names.begin(), names.end(), ".hidden") != names.end());

    model2.update(td.path, false, "");
    assert(model2.get_item().left == "adir");
    model2.process_key("l", "");
    assert(model2.get_item().left == "inside.txt");
    model2.process_key("h", "");
    assert(model2.get_item().left == "adir");

    auto prev1 = model2.get_preview("adir", 7);
    auto prev2 = model2.get_preview("adir", 99);
    assert(prev1.size() == static_cast<size_t>(1));
    assert(prev1[0] == "stub-preview");
    assert(prev1[0] == "stub-preview");
    // Same path key is cached, so the second height is intentionally ignored.

    // Print header of overall result.
    std::cout                                         << std::endl;
    std::cout << "=============================="     << std::endl;
    std::cout << "\033[33mOVERALL TEST RESULT\033[0m" << std::endl;

    // Print test result.
    if (passed) { std::cout << "\033[32mPASSED\033[0m" << std::endl; }
    else        { std::cout << "\033[31mFAILED\033[0m" << std::endl; }

    std::cout << "test finished" << std::endl;

    return EXIT_SUCCESS;

}   // }}}

