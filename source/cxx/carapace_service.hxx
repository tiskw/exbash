////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: carapace_service.hxx                                                        ///
///                                                                                              ///
/// A class that provides shell-style tab completion by communicating with "carapace" process    ///
/// running in the background.                                                                   ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CARAPACE_SERVICE_HXX
#define CARAPACE_SERVICE_HXX

// Include the headers of custom modules.
#include "dtypes.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////////////////////////////////

class CarapaceService
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

         CarapaceService(void); // Constructor: spawns a bash child process and initialises bash-completion.
        ~CarapaceService(void); // Destructor : closes the pipes and reaps the child process.

        // NOTE: This class should be non-copyable and non-movable, because this class owns raw file descriptors.
        CarapaceService(const CarapaceService&)              = delete;
        CarapaceService& operator = (const CarapaceService&) = delete;
        CarapaceService(CarapaceService&&)                   = delete;
        CarapaceService& operator = (CarapaceService&&)      = delete;

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        Generator<Pair<StringView, StringView>> complete(const Vector<StringView>& tokens);
        // Returns a list of completion candidates for the given command-line string.
        //
        // [Args]
        //   tokens (const Vector<StringView>&): [IN] The parsed tokens of the user input.
        //
        // [Returns]
        //   (Generator<Pair<String, String>>): A vector of matching completion strings and their descriptions.

    private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        Map<uint64_t, Vector<Pair<String, String>>> cache;
        // Cache for storing previously computed completion results.
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
