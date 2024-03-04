/**
 * @file sm_client.cpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <algorithm>
#include <iostream>
#include "../inc/sm_client.hpp"

namespace sm
{
    Client::Client()
    {
        client_thread = std::make_unique<std::thread>(std::thread(&Client::clientThread,this));
    }

    Client::~Client()
    {
        thread_stop.store(true, std::memory_order_relaxed);
        client_thread->join();
    }

    void Client::connect(const std::uint8_t address)
    {
        if(server_id == not_connected)
        {
            //create server and select server first
            addServer(address);
        }
        if(servers[server_id].info.status != ServerStatus::Available)
        {
            ping();
            //if ping success -> load info
        }
    }

    void Client::disconnect()
    {
        if(server_id != not_connected)
        {
            servers[server_id].info.status = ServerStatus::Unavailable;
            server_id = not_connected;
        }
    }

    void Client::addServer(const std::uint8_t address)
    {
        auto it = std::find_if(servers.begin(),servers.end(),[]( ServerData& server){return server.info.status == ServerStatus::Unavailable;} );
        if(it != servers.end())
        {
            int index =  std::distance(servers.begin(), it);
            servers[index] = ServerData();
            servers[index].info.addr = address;
            server_id = index;
        }
        else
        {
            servers.push_back(ServerData());
            servers.back().info.addr = address;
            server_id = servers.size() - 1;
        }
    }

    void Client::clientThread()
    {
        using namespace std::chrono_literals;
        while (!thread_stop.load(std::memory_order_relaxed))
        {
            std::this_thread::sleep_for(50ms);
        }
    }

    void Client::ping()
    {
        if(server_id != not_connected)
        {
            std::uint8_t address  = servers[server_id].info.addr;
            std::uint8_t function = static_cast<uint8_t>(modbus::FunctionCodes::undefined);
            std::vector<uint8_t> message{0x00,0x00,0x00,0x00};
            std::uint8_t length;

            switch(modbus_client.getMode())
            {
                case modbus::ModbusMode::rtu:
                    length = rtu_adu_size;
                    break;
                
                case modbus::ModbusMode::ascii:
                    length = ascii_adu_size;
                    break;
                
                default:
                    length = 0;
                    break;
            }
            TaskAttributes attr = 
            {
                .code   = modbus::FunctionCodes::undefined,
                .length = static_cast<size_t>(length + 2) // 1 byte for exception + 1 byte for func + modbus required part
            };
            task_info.task = ClientTasks::ping; 
            request_data = modbus_client.msgCustom(address,function,message);
            createServerRequest(attr);
            task.wait();
            task_info = TaskInfo();
        }
    }

    void Client::createServerRequest(const TaskAttributes& attr)
    {
        task_info.attributes = attr;
        task = std::async(&Client::callServerExchange,this,attr.length);
    }

    void Client::callServerExchange(const size_t resp_length)
    {
        //to be written
        for(auto i = 0; i < request_data.size(); ++i)
        {
            std::printf("0x%x ",request_data[i]);
        }
        std::printf("\n");
    }
}
