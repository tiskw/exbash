////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: dtypes.hxx                                                                  ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DTYPES_HXX
#define DTYPES_HXX

// Include the headers of STL.
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <generator>
#include <regex>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// Include ncurses.
#include <ncurses.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace aliases
////////////////////////////////////////////////////////////////////////////////////////////////////

// Filesystem namespace.
namespace stdfs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Template alias for STL data types
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, std::size_t N>
using Array = std::array<T, N>;

template<typename T>
using Generator = std::generator<T>;

template<typename T, typename U>
using Map = std::unordered_map<T, U>;

template<typename T, typename U>
using Pair = std::pair<T, U>;

template<typename T>
using Set = std::unordered_set<T>;

template<typename... T>
using Tuple = std::tuple<T...>;

template<typename T>
using Vector = std::vector<T>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Type aliases
////////////////////////////////////////////////////////////////////////////////////////////////////

// Color class.
using Colors = Array<Array<uint8_t, 3>, 8>;

// Path class.
using Path = stdfs::path;

// Pointer difference type.
using PtrDiff = std::ptrdiff_t;

// Regular expression class.
using RegEx = std::regex;

// Size type for indexing and sizes.
using SizeType = std::size_t;

// String class.
using String = std::string;

// String view class.
using StringView = std::string_view;

// Map of string to string.
using StrMap = Map<String, String>;

// Time point class for measuring time intervals.
using TimePoint = std::chrono::steady_clock::time_point;

// Window class of ncurses.
using Window = WINDOW*;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Custom data types
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Rect
// Rectangle information.
{
    int32_t x = 0;  // X coordinate (top-left corner).
    int32_t y = 0;  // Y coordinate (top-left corner).
    int32_t w = 0;  // Width.
    int32_t h = 0;  // Height.
};

struct FileItem
// Item of list used in the Curses screen classes.
{
    String  left  = {};        // Left-side string.
    String  right = {};        // Right-side string.
    int32_t attr  = A_NORMAL;  // Curses attribute (color, bold, etc.).
    bool    star  = false;     // True if selected (starred).
   
    //--------------------------------------------------------------------------
    // Custom comparison operators
    //--------------------------------------------------------------------------

    bool operator < (const FileItem& other) const noexcept
    { return left < other.left; }
};

struct LineItem
// Item of list used in the Curses screen classes.
{
    String line = {};    // Line string.
    bool   star = false; // True if selected (starred).

    //--------------------------------------------------------------------------
    // Custom comparison operators
    //--------------------------------------------------------------------------

    bool operator < (const LineItem& other) const noexcept
    { return line < other.line; }
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
