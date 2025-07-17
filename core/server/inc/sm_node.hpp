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

#include "sm_init.hpp"
#include "sm_server.hpp"
#include <cstddef>
#include <cstdint>

namespace sm
{

constexpr std::uint32_t receive_timeout_ms = 1000;

template <typename c, typename t, typename WaitPolicy>
class DataNode
{
public:
    DataNode(std::uint8_t address, std::uint8_t record_size) : server(address, &server_resources), server_resources(record_size, &buffer_control) {}
    void start()
    {
        initializer.initModbusServer(server_resources, server, buffer_control);
        com.init();
        if (com.isConfigured())
        {
            com.readData(buffer.data(), buffer_control.getSize());
        }
    }
    void loop()
    {
        for (;;)
        {
            if (!com.isConfigured())
            {
                break;
            }
            if (com.isBusy() && !timer.isStarted())
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
    ServerInitializer initializer;
    ModbusServer server;
    ServerResources server_resources;
    BufferControl buffer_control;
    std::array<std::uint8_t, modbus::max_adu_size> buffer;
    c com;
    t timer;
    void handleTimeOut()
    {
        if (timer.isDone() && com.isBusy())
        {
            com.flush();
            timer.stop();
            com.readData(buffer.data(), buffer_control.getSize());
        }
    }
    void handleReady()
    {
        if (com.isReady())
        {
            last_error = server.serverTask(buffer.data(), buffer_control.getSize());
            com.sendData(buffer.data(), server.getTransmitBufferSize());
            com.readData(buffer.data(), buffer_control.getSize());
        }
    }
};

} // namespace sm

#endif // SM_NODE_HPP
