////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: cmd_runner.hxx                                                              ///
///                                                                                              ///
/// This file declares the "run_command" function, which runs an external command and gets       ///
/// the returned value as a string.                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CMD_RUNNER_HXX
#define CMD_RUNNER_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Data types
////////////////////////////////////////////////////////////////////////////////////////////////////

enum RunCommandOption : uint8_t
{
    RUN_COMMAND_PLAIN    = 0,       // Run the command without any special options.
    RUN_COMMAND_NO_STRIP = 1 << 0,  // Do not strip the leading and trailing whitespace characters.
    RUN_COMMAND_GETOUT   = 1 << 1,  // Get the output of the command and return it as a string.
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

String run_command(StringView                command,    RunCommandOption option = RUN_COMMAND_PLAIN);
String run_command(const Vector<String>&     cmd_tokens, RunCommandOption option = RUN_COMMAND_PLAIN);
String run_command(const Vector<StringView>& cmd_tokens, RunCommandOption option = RUN_COMMAND_PLAIN);
// Run a external command and get the returned value as a string.
//
// [Args]
//   cmd_tokens   (const Vector<String*>&): [IN] Command and its arguments to run.
//   option       (RunCommandOption)      : [IN] Options for running the command.
//
// [Returns]
//   (String): Return value of the external command.

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
