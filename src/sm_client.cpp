/**
 * @file sm_client.cpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <iostream>
#include <algorithm>
#include "../inc/sm_client.hpp"

namespace sm
{
    Client::Client():task_info(TaskInfo(ClientTasks::undefined,0))
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
            task_info = TaskInfo(ClientTasks::ping,1);
            auto lambda = [this]() 
            {
                q_exchange.push([this]{ping();});
            };
            q_task.push(lambda);
            while(!task_info.done);
            if(servers[server_id].info.status == ServerStatus::Available)
            {
                //if ping success -> load info
            }
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
            while(!q_task.empty())
            {
                q_task.front()();
                while(!q_exchange.empty())
                {
                    try
                    {
                        q_exchange.front()();
                        task.wait();
                    }
                    catch(const std::system_error& e)
                    {
                        std::cerr << e.what() << std::endl;
                    }
                    exchangeCallback();
                    if(task_info.error == true)
                    {
                        std::queue<std::function<void()>> empty;
                        std::swap(q_exchange,empty);
                    }else{q_exchange.pop();}
                }
                q_task.pop();
            }
            std::this_thread::sleep_for(50ms);
        }
    }

    void Client::writeRecord(const std::uint16_t file_id, const std::uint16_t record_id, const std::vector<std::uint8_t>& data)
    {
        if(server_id != not_connected)
        {
            request_data = modbus_client.msgWriteFileRecord(servers[server_id].info.addr,file_id,record_id,data);
            TaskAttributes attr = 
            {
                .code   = modbus::FunctionCodes::write_file,
                // in case of success we expect message with the same length
                .length = request_data.size()
            };
            createServerRequest(attr);
        }
    }

    void Client::readRecord(const std::uint16_t file_id, const std::uint16_t record_id, const std::uint16_t length)
    {
        if(server_id != not_connected)
        {
            request_data = modbus_client.msgReadFileRecord(servers[server_id].info.addr,file_id,record_id,length);
            TaskAttributes attr = 
            {
                .code   = modbus::FunctionCodes::read_file,
                // amount of half words + 1 byte for ref type + 1 byte for data length 
                // + 1 byte for resp length + 1 byte for func + modbus required part
                .length = static_cast<size_t>((length * 2) + 4)
            };
            createServerRequest(attr);
        }
    }

    void Client::writeRegister(const std::uint16_t address, const std::uint16_t value)
    {
        if(server_id != not_connected)
        {
            request_data = modbus_client.msgWriteRegister(servers[server_id].info.addr,address,value);
            TaskAttributes attr = 
            {
                .code   = modbus::FunctionCodes::write_register,
                // in case of success we expect message with the same length
                .length = request_data.size()
            };
            createServerRequest(attr);
        }
    }

    void Client::readRegisters(const std::uint16_t address, const std::uint16_t quantity)
    {
        if(server_id != not_connected)
        {
            request_data = modbus_client.msgReadRegisters(servers[server_id].info.addr,address,quantity);
            TaskAttributes attr = 
            {
                .code   = modbus::FunctionCodes::read_registers,
                // amount of 16 bit registers + 1 byte for length + 1 byte for func + modbus required part
                .length = static_cast<size_t>(getModbusRequriedLength() + (quantity * 2) + 2)
            };
            createServerRequest(attr);
        }
    }

    void Client::ping()
    {
        if(server_id != not_connected)
        {
            std::uint8_t address  = servers[server_id].info.addr;
            std::uint8_t function = static_cast<uint8_t>(modbus::FunctionCodes::undefined);
            std::vector<uint8_t> message{0x00,0x00,0x00,0x00};
            request_data = modbus_client.msgCustom(address,function,message);
            TaskAttributes attr = 
            {
                .code   = modbus::FunctionCodes::undefined,
                // 1 byte for exception + 1 byte for func + modbus required part
                .length = static_cast<size_t>(getModbusRequriedLength() + 2) 
            };
            createServerRequest(attr);
        }
    }
    
    void Client::exchangeCallback()
    {
        ++task_info.counter;
        if(modbus_client.isChecksumValid(responce_data))
        {
            switch(task_info.task)
            {
                case ClientTasks::ping:
                    if(server_id != not_connected){servers[server_id].info.status = ServerStatus::Available;}
                    break;
                
                default:
                    break;
            }
            if(task_info.counter == task_info.num_of_exchanges){task_info.done = true;}
        }
        else
        {
            task_info.error = true;
            task_info.done = true;
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
    
    std::uint8_t sm::Client::getModbusRequriedLength() const
    {
        std::uint8_t length;
        switch(modbus_client.getMode())
        {
            case modbus::ModbusMode::rtu:
                length = modbus::rtu_adu_size;
                break;
            
            case modbus::ModbusMode::ascii:
                length = modbus::ascii_adu_size;
                break;
            
            default:
                length = 0;
                break;
        }
        return length;
    }
}