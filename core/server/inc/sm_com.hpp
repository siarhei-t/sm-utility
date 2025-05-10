/**
 * @file sm_com.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_COM_HPP
#define SM_COM_HPP

#include <atomic>

namespace sm
{

template <class Impl>
class Com
{
public:
    void init()
    {
        configured = static_cast<Impl*>(this)->platformInit();
    }
    void readData(std::uint8_t* data, const size_t amount)
    {
        if(isConfigured())
        {
            ready.store(false, std::memory_order_relaxed);
            static_cast<Impl*>(this)->platformReadData(data,amount);
        }
    }
    void sendData(std::uint8_t* data, const size_t amount)
    {
        if(isConfigured())
        {
            static_cast<Impl*>(this)->platformSendData(data,amount);
        }
    }
    void flush()
    {
        if(isConfigured())
        {
            static_cast<Impl*>(this)->platformFlush();
        }
    }
    [[nodiscard]] bool isConfigured() const { return configured; }
    [[nodiscard]] bool isReady() const { return ready.load(std::memory_order_acquire); }
    [[nodiscard]] bool isBusy() const { return busy.load(std::memory_order_acquire); }
    void setReady() { ready.store(true,std::memory_order_release); }
    void setBusy() { busy.store(true,std::memory_order_release); }

private:
    bool configured = false;
    std::atomic<bool> ready{false};
    std::atomic<bool> busy{false};
};

} // namespace sm

#endif // SM_COM_HPP
