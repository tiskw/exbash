////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ header file: async_comp.hxx                                                              ///
///                                                                                              ///
/// Asynchronous completion module for EditHelper in ExBash.                                     ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ASYNC_COMP_HXX
#define ASYNC_COMP_HXX

// Include the headers of custom modules.
#include "config.hxx"
#include "dtypes.hxx"
#include "edit_helper.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Class definition
////////////////////////////////////////////////////////////////////////////////////////////////////

class AsyncComp
{
    public:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Constructors and destructors
        ////////////////////////////////////////////////////////////////////////////////////////////

         AsyncComp(uint16_t rows, uint16_t cols, const ExBashConfig& cfg);
        ~AsyncComp(void);

        // Delete copy constructor and copy assignment operator to prevent copying of this class,
        // because it manages a worker thread and shared resources that should not be duplicated.
        AsyncComp(const AsyncComp&)            = delete;
        AsyncComp& operator=(const AsyncComp&) = delete;

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        Vector<String> get_completion_result(void);
        // Same interface as before; picks up results from the worker thread.
        //
        // [Returns]
        //   (Vector<String>): Array of lines (strings) for showing completion candidates to users.

        int get_wakeup_fd(void) const noexcept;
        // Returns the read end of the wakeup pipe. The main thread can pass this fd to
        // select()/poll() to be notified immediately when a new completion result is available.
        //
        // [Returns]
        //   (int): File descriptor for the read end of the wakeup pipe, or -1 on error.

        void launch_async_completion(StringView lhs);
        // Post a new completion task to the worker thread.
        //
        // [Args]
        //   lhs (StringView): [IN] Current left-hand-side string.

     private:

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member functions
        ////////////////////////////////////////////////////////////////////////////////////////////

        void worker_loop(void);
        // Worker thread loop that waits for completion tasks and executes them.

        ////////////////////////////////////////////////////////////////////////////////////////////
        // Private member variables
        ////////////////////////////////////////////////////////////////////////////////////////////

        std::thread worker;
        // Worker thread that performs completion tasks.

        bool flag_end;
        // Flag to signal the worker thread to end.

        std::condition_variable cv_task;
        // Condition variable for controlling the worker thread.

        bool   has_task;
        String lhs_task;
        // Task side (written by main thread, read by worker).

        std::atomic<bool> has_result;
        String            lhs_result;
        Vector<String>    buf_result;
        // Result side (written by worker, read by main thread).

        std::mutex mtx;
        // Mutex for synchronizing access to shared variables (task and result).

        uint32_t gen_id_task;
        // Generation IDs for tasks, used to track the freshness of results.

        int wakeup_pipe[2];
        // Pipe used to wake up the main thread when a new result is ready.
        // wakeup_pipe[0]: read end (passed to getch via get_wakeup_fd).
        // wakeup_pipe[1]: write end (written by the worker thread).

        Vector<String> clines;
        // Cached display lines.

        EditHelper helper;
        // Helper for computing completion candidates (used only in the worker thread).
};

#endif

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
