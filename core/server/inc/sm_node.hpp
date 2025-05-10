/**
 * @file sm_node.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_NODE_HPP
#define SM_NODE_HPP

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <cstdio>
#include "sm_server.hpp"

namespace sm
{

constexpr std::uint32_t receive_timeout_ms = 1000;

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

template<typename c, typename t, typename WaitPolicy> class DataNode
{
public:
    DataNode(std::uint8_t address, std::uint8_t record_size) : server(address,record_size) {}
    void start()
    {
        com.init();
        if(com.isConfigured())
        {
            com.readData(buffer.data(),server.getReceiveBufferSize());
        }
    }
    void loop()
    {
        for(;;)
        {
            if(!com.isConfigured()) { break; }
            if(com.isBusy() && !timer.isStarted())
            { 
                timer.setTimeout(receive_timeout_ms);
                timer.start();
            }
            handleTimeOut();
            handleReady();
            WaitPolicy::wait();
        }
    }
    std::uint8_t* getBufferPtr() { return buffer.data(); };

private:
    ServerExceptions last_error = ServerExceptions::no_error;
    ModbusServer server;
    std::array<std::uint8_t, modbus::max_adu_size> buffer;
    c com;
    t timer;
    void handleTimeOut()
    {
        if(timer.isDone() && com.isBusy())
        {
            com.flush();
            timer.stop();
            com.readData(buffer.data(),server.getReceiveBufferSize());
        }
    }
    void handleReady()
    {
        if(com.isReady())
        {
            last_error = server.serverTask(buffer.data(), server.getReceiveBufferSize());
            com.sendData(buffer.data(),server.getTransmitBufferSize());
            com.readData(buffer.data(),server.getReceiveBufferSize());
        }
    }
};

} // namespace sm

#endif // SM_NODE_HPP
