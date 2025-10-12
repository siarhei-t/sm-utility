#ifndef SM_TIMER_HPP
#define SM_TIMER_HPP

#include <atomic>
#include <cstdint>

namespace sm
{
/**
 * @brief Base CRTP timer class template.
 *
 * The `Timer` class provides generic timer control logic and delegates
 * platform-specific implementation details to the derived class (`Impl`)
 * using the Curiously Recurring Template Pattern (CRTP).
 *
 * @tparam Impl Derived class implementing platform-specific behavior.
 *
 * ### Required methods in `Impl`:
 * - `void platformStart();` — start the hardware or platform-specific timer.
 * - `void platformStop();` — stop the hardware or platform-specific timer.
 *
 */
template <class Impl>
class Timer
{
public:
    /**
     * @brief Starts the timer.
     *
     * If the timer is already running, it is stopped first.
     * Then calls `platformStart()` implemented in the derived class.
     */
    void start()
    {
        if (started)
        {
            stop();
        }
        static_cast<Impl*>(this)->platformStart();
        done.store(false, std::memory_order_release);
        started.store(true, std::memory_order_release);
    }

    /**
     * @brief Stops the timer.
     *
     * Calls `platformStop()` from the derived implementation.
     * Resets the `done` flag afterwards.
     */
    void stop()
    {
        static_cast<Impl*>(this)->platformStop();
        done.load(std::memory_order_acquire);
        done.store(false, std::memory_order_release);
    }

    /**
     * @brief Checks if the timer is currently running.
     * @return true if the timer is running, false otherwise.
     */
    bool isStarted() const { return started.load(std::memory_order_acquire); }

    /**
     * @brief Checks if the timer has completed its operation.
     * @return true if the timer has finished, false otherwise.
     */
    bool isDone() const { return done.load(std::memory_order_acquire); }

    /**
     * @brief Marks the timer as completed.
     *
     * Typically called by the platform-specific code when
     * the timer reaches the timeout condition.
     */
    void setDone() const { done.store(true, std::memory_order_release); }

    /**
     * @brief Sets the timeout duration in milliseconds.
     *
     * Can only be called when the timer is not running.
     * @param timeout Timeout duration in milliseconds.
     */
    void setTimeout(const std::uint32_t timeout)
    {
        if (!started.load(std::memory_order_acquire))
        {
            timeout_ms = timeout;
        }
    }

    /**
     * @brief Returns the currently configured timeout value.
     * @return Timeout duration in milliseconds.
     */
    std::uint32_t getTimeout() const { return timeout_ms; }

private:
    std::atomic<bool> done{false};    ///< Completion flag.
    std::atomic<bool> started{false}; ///< Indicates whether the timer is running.
    std::uint32_t timeout_ms = 0;     ///< Timeout duration in milliseconds.
};

} // namespace sm

#endif // SM_TIMER_HPP
