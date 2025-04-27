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

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <cstdio>
#include "../inc/sm_server.hpp"

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
        started = true;
    }
    void stop()
    {
        static_cast<Impl*>(this)->platformStop();
        started = false;
        done.store(false, std::memory_order_release);
    }
    bool isStarted() const { return started; }
    bool isDone() const { return done.load(std::memory_order_acquire); }
    void setDone() const { done.store(true,std::memory_order_release); }
    void setTimeout(const std::uint32_t timeout)
    {
        if(!started)
        {
            timeout_ms = timeout; 
        }
    }
    std::uint32_t getTimeout() const { return timeout_ms; }

private:
    std::atomic<bool> done{false};
    bool started = false; 
    std::uint32_t timeout_ms = 0;
};

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

template<typename c, typename t> class DataNode
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
            //start receiver timeout timer
            if(com.isBusy() && !timer.isStarted())
            {
                timer.start();
            }
            // port flush in case of timeout
            if(timer.isDone() && com.isBusy())
            {
                com.flush();
                timer.stop();
                com.readData(buffer.data(),server.getReceiveBufferSize());
            }
            // data process
            if(com.isReady())
            {
                last_error = server.serverTask(buffer.data(), server.getReceiveBufferSize());
                com.sendData(buffer.data(),server.getTransmitBufferSize());
                com.readData(buffer.data(),server.getReceiveBufferSize());
            }
        }
    }
    std::uint8_t* getBufferPtr() { return buffer.data(); };

private:
    ServerExceptions last_error = ServerExceptions::no_error;
    ModbusServer server;
    std::array<std::uint8_t, modbus::max_adu_size> buffer;
    c com;
    t timer;
};

} // namespace sm

#endif // SM_COM_HPP
