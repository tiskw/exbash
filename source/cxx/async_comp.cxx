////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: async_comp.cxx                                                              ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "async_comp.hxx"

// Include STL headers.
#include <condition_variable>
#include <mutex>
#include <thread>

// Include POSIX headers.
#include <fcntl.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// AsyncComp: Constructors and destructors
////////////////////////////////////////////////////////////////////////////////////////////////////

AsyncComp::AsyncComp(uint16_t rows, uint16_t cols, const ExBashConfig& cfg) : flag_end(false), has_task(true), gen_id_task(0), helper(rows, cols, cfg)
{   // {{{

    // NOTE: The member variable 'has_task' is initialized to true to ensure the worker thread
    //       starts immediately (in the most cases the user input may be an empty string).

    // Initialize the result flag.
    this->has_result.store(false, std::memory_order_release);

    // Create a non-blocking pipe used to wake up the main thread when a new result is ready.
    if (pipe2(this->wakeup_pipe, O_CLOEXEC | O_NONBLOCK) != 0)
        this->wakeup_pipe[0] = this->wakeup_pipe[1] = -1;

    // Start a single persistent worker thread.
    this->worker = std::thread([this]() { this->worker_loop(); });

}   // }}}

AsyncComp::~AsyncComp(void)
{   // {{{

    // Scoped locking pattern for accessing shared variables safely.
    { std::lock_guard<std::mutex> lock(this->mtx);

        // Signal the worker thread to stop and exit.
        this->flag_end = true;
    }

    // Wake the worker thread (in case it's waiting) and join it.
    this->cv_task.notify_one();
    this->worker.join();

    // Close the wakeup pipe.
    if (this->wakeup_pipe[0] >= 0) { close(this->wakeup_pipe[0]); }
    if (this->wakeup_pipe[1] >= 0) { close(this->wakeup_pipe[1]); }

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// AsyncComp: Member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

Vector<String> AsyncComp::get_completion_result(void)
{   // {{{

    // If no result is ready, return the last result.
    if (not this->has_result.load(std::memory_order_acquire))
        return this->clines;

    // Scoped locking pattern for accessing shared variables safely.
    { std::lock_guard<std::mutex> lock(this->mtx);

        if (this->has_result.load(std::memory_order_relaxed))
        {
            // Move the result to the output variable (avoid copying).
            this->clines = std::move(this->buf_result);

            // Mark result consumed.
            this->has_result.store(false, std::memory_order_relaxed);
        }
    }

    return this->clines;

}   // }}}

int AsyncComp::get_wakeup_fd(void) const noexcept
{   // {{{

    return this->wakeup_pipe[0];

}   // }}}

void AsyncComp::launch_async_completion(StringView lhs)
{   // {{{

    // Scoped locking pattern for accessing shared variables safely.
    { std::lock_guard<std::mutex> lock(this->mtx);

        // Skip launching the new task if the given left-hand-side string is the same as the latest
        // processed left-hand-side string.
        if (this->lhs_result != lhs)
        {
            this->lhs_task = String(lhs);
            this->has_task = true;

            //
            ++this->gen_id_task;
        }
    }

    // Wake the worker thread (no-op if already running).
    this->cv_task.notify_one();

}   // }}}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private member functions
////////////////////////////////////////////////////////////////////////////////////////////////////

void AsyncComp::worker_loop(void)
{   // {{{

    while (true)
    {
        String   lhs;
        uint32_t gen_id_result;

        // Scoped locking pattern for accessing shared variables safely.
        { std::unique_lock<std::mutex> lock(this->mtx);

            // Wait until a new task is posted or a stop signal is received.
            this->cv_task.wait(lock, [this](){ return (this->has_task or this->flag_end); });

            // Exit if stop signal is received.
            if (this->flag_end) break;

            // Move the task to a local variable and mark it as consumed.
            lhs = std::move(this->lhs_task);
            this->has_task = false;

            // Capture the generation ID for this task.
            gen_id_result = this->gen_id_task;
        }

        // Perform completion outside the lock (allows new tasks to be posted while running).
        Vector<String> computed = this->helper.candidate(lhs);

        // Scoped locking pattern for accessing shared variables safely.
        { std::lock_guard<std::mutex> lock(this->mtx);

            // Store the result to the current buffer only if the task generation ID is the latest one.
            if (gen_id_result == this->gen_id_task)
            {
                this->buf_result = std::move(computed);
                this->lhs_result = std::move(lhs);
                this->has_result = true;
            }
        }

        // Notify the main thread that a new result is ready by writing a byte to the wakeup pipe.
        // O_NONBLOCK ensures this never blocks even if the pipe buffer is full.
        // The return value is intentionally ignored: a failed write is non-fatal since the
        // main thread will pick up the result on the next poll cycle anyway.
        if (this->wakeup_pipe[1] >= 0)
        {
            constexpr char byte = '\x01';

            // Write something to the wakeup pipe to wake up the "TermUserIF::getch".
            [[maybe_unused]] auto _ = write(this->wakeup_pipe[1], &byte, 1);
        }
    }

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
