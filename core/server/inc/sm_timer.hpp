/**
 * @file sm_timer.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_TIMER_HPP
#define SM_TIMER_HPP

#include <cstdint>
#include <atomic>

namespace sm
{

template <class Impl>
class Timer
{
public:
    void start()
    {
        if(started)
        {
            stop();
        }
        static_cast<Impl*>(this)->platformStart();
        done.store(false,std::memory_order_release);
        started.store(true,std::memory_order_release);
    }
    void stop()
    {
        static_cast<Impl*>(this)->platformStop();
        done.load(std::memory_order_acquire);
        done.store(false, std::memory_order_release);
    }
    bool isStarted() const { return started.load(std::memory_order_acquire); }
    bool isDone() const { return done.load(std::memory_order_acquire); }
    void setDone() const { done.store(true,std::memory_order_release); }
    void setTimeout(const std::uint32_t timeout)
    {
        if(!started.load(std::memory_order_acquire)) { timeout_ms = timeout; }
    }
    std::uint32_t getTimeout() const { return timeout_ms; }

private:
    std::atomic<bool> done{false};
    std::atomic<bool> started{false};
    std::uint32_t timeout_ms = 0;
};

} // namespace sm

#endif // SM_TIMER_HPP
