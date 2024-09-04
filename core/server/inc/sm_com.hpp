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
#include "../inc/sm_server.hpp"

namespace sm
{

class Timer
{

public:
    std::atomic<bool> done{false};
    void start()
    {
        if(started)
        {
            stop();
        }
        platformStart();
        started = true;
    }
    void stop()
    {
        started = false;
        done.store(false, std::memory_order_relaxed);
        platformStop();
    }
    bool isStarted() const { return started; }
    void setTimeout(const std::uint32_t timeout)
    {
        if(!started)
        {
            timeout_ms = timeout; 
        }
    }
    std::uint32_t getTimeout() const { return timeout_ms; }

private:
    bool started = false; 
    std::uint32_t timeout_ms = 0; 
    void platformStart()
    {
        done.store(true, std::memory_order_relaxed);
    };
    void platformStop()
    {
        done.store(false, std::memory_order_relaxed);
    };
};

class Com
{
public:
    std::atomic<bool> data_ready{false};
    std::atomic<bool> data_in_progress{false};
    void startReadData(std::uint8_t data[], const size_t amount);
    void startSendData(std::uint8_t data[], const size_t amount);
    void flush(){ platformFlush(); }

private:
    void platformFlush(){}
    void platformReadData(std::uint8_t data[], const size_t amount)
    {
        (void)(data);
        (void)(amount);
    }
    void platformSendData(std::uint8_t data[], const size_t amount)
    {
        (void)(data);
        (void)(amount);
    }
};

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define, typename c = Com, typename t = Timer> class DataNode
{
public:
    DataNode(std::uint8_t address) : server(address){}
    void start();
    void loop(); 
    std::uint8_t* getBufferPtr() { return buffer.data(); };

private:
    ServerExceptions last_error = ServerExceptions::no_error;
    ModbusServer<amount_of_regs,amount_of_files,record_define> server;
    std::array<std::uint8_t, modbus::max_adu_size> buffer;
    c com;
    t timer;
};

} // namespace sm

#endif // SM_COM_HPP
