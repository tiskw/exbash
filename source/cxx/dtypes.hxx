////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: dtypes.hxx                                                                  ///
///                                                                                              ///
/// This header file defines data types, including type name aliases, used in ExBash.            ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DTYPES_HXX
#define DTYPES_HXX

// Include the headers of STL.
#include <cstdint>
#include <deque>
#include <filesystem>
#include <generator>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace aliases
////////////////////////////////////////////////////////////////////////////////////////////////////

// Filesystem namespace.
namespace stdfs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Custom data types
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Size
// Data type for size of the terminal or drawing area.
{
    uint16_t cols;
    uint16_t rows;
};

enum class CompType
// Data type for completion type.
{
    NONE      =  0,
    BASHCOMP  =  1,
    CARAPACE  =  2,
    COMMAND   =  3,
    GREP      =  4,
    OPTION    =  5,
    PATH      =  6,
    PREVIEW   =  7,
    SHELL     =  8,
    SUBCMD    =  9,
    SC_AND_BC = 10,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Template alias for STL data types
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
using Deque = std::deque<T>;

template<typename T>
using Generator = std::generator<T>;

template<typename T, typename U>
using Map = std::unordered_map<T, U>;

template<typename T>
using OrderedSet = std::set<T>;

template<typename T>
using Optional = std::optional<T>;

template<typename T, typename U>
using Pair = std::pair<T, U>;

template<typename T>
using Set = std::unordered_set<T>;

template<typename... T>
using Tuple = std::tuple<T...>;

template<typename T>
using UniqPtr = std::unique_ptr<T>;

template<typename T>
using Vector = std::vector<T>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Type aliases
////////////////////////////////////////////////////////////////////////////////////////////////////

// Path class.
using Path = stdfs::path;

// Pointer difference type.
using PtrDiff = std::ptrdiff_t;

// Regular expression class.
using RegEx = std::regex;

// Signed size type.
using SignedSizeType = ssize_t;

// Size type.
using SizeType = std::size_t;

// String class.
using String = std::string;

// String view class.
using StringView = std::string_view;

// String iterator class.
using StringIter = std::string::iterator;
using StringViewIter = std::string_view::iterator;

// String constant iterator class.
using StringConstIter = std::string::const_iterator;
using StringViewConstIter = std::string_view::const_iterator;

// Completion type.
using Completion = Tuple<Vector<String>, CompType, String>;

// Map type for caching completion candidates.
using CandCacheMap = Map<SizeType, Pair<Vector<Pair<String, String>>, Vector<String>>>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Data type alias with transparent hash function
////////////////////////////////////////////////////////////////////////////////////////////////////

struct string_hash
{
    // This hash function is designed to be transparent, allowing it to accept
    // both std::string and std::string_view as keys without requiring conversions.
    using is_transparent = void;

    size_t operator()(StringView sv) const {
        return std::hash<StringView>{}(sv);
    }

    size_t operator()(const String& s) const {
        return std::hash<StringView>{}(StringView(s));
    }

    size_t operator()(const char* s) const {
        return std::hash<StringView>{}(StringView(s));
    }
};

// Map and set types with transparent hash function for string keys.
using StringMap = std::unordered_map<String, String, string_hash, std::equal_to<void>>;
using StringSet = std::unordered_set<String, string_hash, std::equal_to<void>>;
using StrVecMap = std::unordered_map<String, Vector<String>, string_hash, std::equal_to<void>>;

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
