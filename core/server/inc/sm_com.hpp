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
    void start()
    {
        done.store(false, std::memory_order_relaxed);
        platformStart();
    }
    
    std::atomic<bool> done{false};

private:
    void platformStart()
    {
        done.store(true, std::memory_order_relaxed);
    };

};

class Com
{
public:
    void startReadData(std::uint8_t data[], const size_t amount);
    void startSendData(std::uint8_t data[], const size_t amount);
    std::atomic<bool> data_ready{false};

private:
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

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define, typename c = Com> class DataNode
{
public:
    DataNode(std::uint8_t address) : server(address){}
    void start();
    void loop(); 
    std::uint8_t* getBufferPtr() { return buffer.data(); };

private:
    std::uint8_t rx_length = 0;
    std::uint8_t tx_length = 0;
    ModbusServer<amount_of_regs,amount_of_files,record_define> server;
    std::array<std::uint8_t, modbus::max_adu_size> buffer;
    c com;
};

} // namespace sm

#endif // SM_COM_HPP
