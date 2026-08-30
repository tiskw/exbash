////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: read_cmd.hxx                                                                ///
///                                                                                              ///
/// C++ implementation of readcmd, an alternative of readline.                                   ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef READ_CMD_HXX
#define READ_CMD_HXX

// Include the headers of custom modules.
#include "config.hxx"
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Data types
////////////////////////////////////////////////////////////////////////////////////////////////////

struct ReadCmdOut
{
    // Output of the "readcmd" function.
    // This struct is used to return multiple values from the "readcmd" function,
    // including the left hand side, right hand side, stop key, and rest of input.

    String lhs, rhs;
    // Left and right hand side of the editing buffer.

    String stop;
    // Stop key that stops the read loop in the "readcmd" function.

    StringView input;
    // Rest of the user input that is not processed in the "readcmd" function.

    ReadCmdOut(StringView lhs, StringView rhs, StringView stop, StringView input)
     : lhs(lhs), rhs(rhs), stop(stop), input(input) { /* Do nothing, initializer lists only. */ }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Published variables and functions
////////////////////////////////////////////////////////////////////////////////////////////////////

ReadCmdOut readcmd(StringView lhs_ini, StringView rhs_ini, const Deque<String>& hists, StringView editor_name, StringView inputs, const ExBashConfig& cfg);
// Read user input with rich interface.
//
// [Args]
//   lhs_ini     (StringView)          : [IN] Initial value of left hand side string.
//   rhs_ini     (StringView)          : [IN] Initial value of right hand side string.
//   hists       (const Deque<String>&): [IN] History strings.
//   editor_name (StringView)          : [IN] Editor mode ("emacs" or "vi").
//   inputs      (StringView)          : [IN] User input (for debugging).
//   cfg         (const ExBashConfig&) : [IN] Config instance.
//
// [Returns]
//   (ReadCmdOut): Left and right hand side, stop key, and rest of input.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
