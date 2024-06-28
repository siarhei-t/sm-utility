/**
 * @file sm_client.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sm_client.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace sm
{

Client::~Client()
{
    thread_stop.store(true, std::memory_order_relaxed);
    client_thread.join();
}

void Client::getServerData(ServerData& data)
{
    if (server_id != not_connected)
    {
        data = servers[server_id];
    }
    else
    {
        data = ServerData();
    }
}

int Client::getActualTaskProgress() const 
{ 
    return (task_info.counter * 100)/task_info.num_of_exchanges; 
}

std::error_code Client::start(std::string device)
{
    task_info.error_code = std::error_code();
    if (serial_port.getState() != sp::PortState::Open)
    {
        task_info.error_code = serial_port.open(device);
    }
    else
    {
        if (device != serial_port.getPath())
        {
            serial_port.close();
            task_info.error_code = serial_port.open(device);
        }
    }
    return task_info.error_code;
}

void Client::stop()
{
    disconnect();
    if (serial_port.getState() == sp::PortState::Open)
    {
        serial_port.close();
    }
}

std::error_code Client::configure(sp::PortConfig config)
{
    task_info.error_code = serial_port.setup(config);
    return task_info.error_code;
}

std::error_code Client::connect(const std::uint8_t address, const std::uint8_t gateway_address)
{
    //flush port buffer first
    serial_port.port.flushPort();
    
    // server is not selected yet or current server has different address
    if((server_id == not_connected) || (servers[server_id].info.addr != address))
    {
        addServer(address,gateway_address);
    }
    
    // (1) ping server, expected answer with exception type 1
    if (servers[server_id].info.status != ServerStatus::Available)
    {
        task_info.error_code = taskPing(address);
        if (task_info.error_code)
        {
            disconnect();
            return task_info.error_code;
        }
    }
    
    // (2) load all registers (they will fit into one message)
    if (servers[server_id].info.status == ServerStatus::Available)
    {
        // download registers
        task_info.reset(ClientTasks::regs_read, 1);
        q_task.push([this]() { q_exchange.push([this] { readRegisters(modbus::holding_regs_offset, amount_of_regs); }); });
        while (!task_info.done);
        if (task_info.error_code)
        {
            disconnect();
            return task_info.error_code;
        }
    }
    // (3) download file with bootloader information
    // (3.1) prepare to read
    task_info.reset(ClientTasks::reg_write, 1);
    q_task.push([this]() { q_exchange.push([this] { writeRegister(static_cast<std::uint16_t>(ServerRegisters::file_control), file_read_prepare); }); });
    while (!task_info.done);
    if (task_info.error_code)
    {
        disconnect();
        return task_info.error_code;
    }
    // (3.2) file reading
    task_info.reset();
    q_task.push([this]() { readFile(ServerFiles::server_metadata); });
    while (!task_info.done);
    
    if (task_info.error_code)
    {
        disconnect();
        return task_info.error_code;
    }
    return task_info.error_code;
}

void Client::disconnect()
{
    if (server_id != not_connected)
    {
        //flush port buffer first
        serial_port.port.flushPort();
        responce_data.clear();
        servers[server_id].info.status = ServerStatus::Unavailable;
        server_id = not_connected;
    }
}

std::error_code Client::eraseApp()
{
    //flush port buffer first
    serial_port.port.flushPort();
    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    if (server_id != not_connected)
    {
        if (servers[server_id].info.status == ServerStatus::Available)
        {
            // (1) erase request
            task_info.reset(ClientTasks::reg_write, 1);
            q_task.push([this]() { q_exchange.push([this] { writeRegister(static_cast<std::uint16_t>(ServerRegisters::app_erase), app_erase_request); }); });
            // we may have here timeout problem because flash erase take a lot of time in some cases, add additional logic for this case in future
            while (!task_info.done);
            if (task_info.error_code)
            {
                disconnect();
                return task_info.error_code;
            }
            // (2) read register with status information
            task_info.reset(ClientTasks::regs_read, 1);
            q_task.push([this]() { q_exchange.push([this] { readRegisters(modbus::holding_regs_offset, amount_of_regs); }); });
            while (!task_info.done);
            if (task_info.error_code)
            {
                disconnect();
                return task_info.error_code;
            }
            // (3) cheking if erase was performed succesfully 
            if (servers[server_id].regs[static_cast<int>(ServerRegisters::boot_status)] != static_cast<std::uint16_t>(BootloaderStatus::empty))
            {
                task_info.error_code = make_error_code(ClientErrors::internal);
            }
        }
    }
    return task_info.error_code;
}

std::error_code Client::uploadApp(const std::string path_to_file)
{
    //flush port buffer first
    serial_port.port.flushPort();
    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    if (server_id != not_connected)
    {
        if (servers[server_id].info.status == ServerStatus::Available)
        {
            BootloaderStatus bootloader_status = static_cast<BootloaderStatus>(servers[server_id].regs[static_cast<int>(ServerRegisters::boot_status)]);
            // (1) erase app file if we have some another state except BootloaderStatus::empty
            if (bootloader_status != BootloaderStatus::empty)
            {
                (void)eraseApp();
                if (task_info.error_code)
                {
                    disconnect();
                    return task_info.error_code;
                }
            }
            
            std::uint8_t block_size = servers[server_id].regs[static_cast<int>(ServerRegisters::record_size)];
            // (2) load full firmware file into vector
            if (file.fileExternalWriteSetup(static_cast<std::uint16_t>(ServerFiles::application),path_to_file, block_size))
            {
                // (3) send new file size
                task_info.reset(ClientTasks::reg_write, 1);
                q_task.push([this]()
                            { q_exchange.push([this] { writeRegister(static_cast<std::uint16_t>(ServerRegisters::app_size), file.getNumOfRecords()); }); });
                while (!task_info.done);
                if (task_info.error_code)
                {
                    disconnect();
                    return task_info.error_code;
                }
                // (4) prepare to write
                task_info.reset(ClientTasks::reg_write, 1);
                q_task.push([this]()
                            { q_exchange.push([this] { writeRegister(static_cast<std::uint16_t>(ServerRegisters::file_control), file_write_prepare); }); });
                while (!task_info.done);
                if (task_info.error_code)
                {
                    disconnect();
                    return task_info.error_code;
                }
                // (5) file sending
                task_info.reset();
                q_task.push([this, path_to_file]() { writeFile(ServerFiles::application); });
                while (!task_info.done);
                if (task_info.error_code)
                {
                    disconnect();
                    return task_info.error_code;
                }
                // (6) read status back
                task_info.reset(ClientTasks::regs_read, 1);
                q_task.push([this]() { q_exchange.push([this] { readRegisters(modbus::holding_regs_offset, amount_of_regs); }); });
                while (!task_info.done);
                if (task_info.error_code)
                {
                    disconnect();
                    return task_info.error_code;
                }
                // (7) check status again
                BootloaderStatus bootloader_status = static_cast<BootloaderStatus>(servers[server_id].regs[static_cast<int>(ServerRegisters::boot_status)]);
                if (bootloader_status != BootloaderStatus::ready)
                {
                    task_info.error_code = make_error_code(ClientErrors::internal);
                }
            }
            else
            {
                task_info.error_code = make_error_code(ClientErrors::internal);
            }
        }
    }
    return task_info.error_code;
}

std::error_code Client::startApp() 
{
    //flush port buffer first
    serial_port.port.flushPort();
    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    if (server_id != not_connected)
    {
        if (servers[server_id].info.status == ServerStatus::Available)
        {
            // write start to start app register
            task_info.reset(ClientTasks::app_start, 1);
            q_task.push([this]() { q_exchange.push([this] { writeRegister(static_cast<std::uint16_t>(ServerRegisters::app_start), app_start_request); }); });
            while (!task_info.done);
        }
    }
    return task_info.error_code;
}

std::error_code Client::taskPing(const std::uint8_t addr)
{
    static bool recurced = false;
    auto lambda_ping = [this](const std::uint8_t address)
    {
        std::uint8_t function = static_cast<uint8_t>(modbus::FunctionCodes::undefined);
        std::vector<uint8_t> message{0x00, 0x00, 0x00, 0x00};
        request_data = modbus_client.msgCustom(address, function, message);
        // 1 byte for exception + 1 byte for func + modbus required part
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::undefined, static_cast<size_t>(modbus_client.getRequriedLength() + 2));
        createServerRequest(attr);
    };
    
    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    auto it = std::find_if(servers.begin(), servers.end(), [addr](ServerData& server) { return server.info.addr == addr; });
    if (it != servers.end())
    {
        int index = std::distance(servers.begin(), it);
        //we are trying to reach this server through the gateway, perform gateway setup first
        if((servers[index].info.gateway_addr != 0) && !recurced)
        {
            recurced = true;
            std::uint16_t expected_length = modbus_client.getRequriedLength() + 2;
            std::uint16_t control_reg = static_cast<std::uint16_t>(ServerRegisters::gateway_buffer_size);
            auto error = taskWriteRegister(servers[index].info.gateway_addr,control_reg,expected_length);
            if(error)
            {
                task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
                recurced = false;
                return task_info.error_code;
            }
        }
        //direct address ping
        task_info.reset(ClientTasks::ping, 1);
        q_task.push([this,lambda_ping,addr]() { q_exchange.push([lambda_ping,addr] { lambda_ping(addr); }); });
        while (!task_info.done);
    }
    recurced = false;
    return task_info.error_code;
}

std::error_code Client::taskWriteRegister(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t value)
{
    static bool recurced = false;
    auto lambda_write_reg = [this](const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t value)
    {
        request_data = modbus_client.msgWriteRegister(dev_addr, reg_addr, value);
        // in case of success we expect message with the same length
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::write_register, request_data.size());
        createServerRequest(attr);
    };
    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    auto it = std::find_if(servers.begin(), servers.end(), [dev_addr](ServerData& server) { return server.info.addr == dev_addr; });
    if (it != servers.end())
    {
        int index = std::distance(servers.begin(), it);
        //we are trying to reach this server through the gateway, perform gateway setup first
        if((servers[index].info.gateway_addr != 0) && !recurced)
        {
            recurced = true;
            std::uint16_t expected_length = modbus_client.getRequriedLength() + 4;
            std::uint16_t control_reg = static_cast<std::uint16_t>(ServerRegisters::gateway_buffer_size);
            auto error = taskWriteRegister(servers[index].info.gateway_addr,control_reg,expected_length);
            if(error)
            {
                task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
                recurced = false;
                return task_info.error_code;
            }
        }
        //direct register write
        task_info.reset(ClientTasks::reg_write, 1);
        q_task.push([this,lambda_write_reg,dev_addr,reg_addr,value]() { q_exchange.push([lambda_write_reg,dev_addr,reg_addr,value] { lambda_write_reg(dev_addr,reg_addr,value); }); });
        while (!task_info.done);
    }
    recurced = false;
    return task_info.error_code;
}

void Client::addServer(const std::uint8_t addr, const std::uint8_t gateway_addr)
{
    auto it = std::find_if(servers.begin(), servers.end(), [addr](ServerData& server) { return server.info.addr == addr; });
    if (it == servers.end())
    {
        servers.push_back(ServerData());
        servers.back().info.addr = addr;
        servers.back().info.gateway_addr = gateway_addr;
        server_id = servers.size() - 1;
    }
}

void Client::clientThread()
{
    using namespace std::chrono_literals;
    while (!thread_stop.load(std::memory_order_relaxed))
    {
        while (!q_task.empty())
        {
            bool error_in_task = false;
            q_task.front()();
            while (!q_exchange.empty())
            {
                try
                {
                    q_exchange.front()();
                    task.wait();
                }
                catch (const std::system_error& e)
                {
                    error_in_task = true;
                    task_info.error_code = e.code();
                }
                if(!error_in_task)
                {
                    exchangeCallback();
                }
                if (task_info.error_code.value())
                {
                    std::queue<std::function<void()>> empty;
                    std::swap(q_exchange, empty);
                }
                else
                {
                    q_exchange.pop();
                }
            }
            q_task.pop();
        }
        std::this_thread::sleep_for(50ms);
    }
}

void Client::writeRecord(const std::uint16_t file_id, const std::uint16_t record_id, const std::vector<std::uint8_t>& data)
{
    if (server_id != not_connected)
    {
        request_data = modbus_client.msgWriteFileRecord(servers[server_id].info.addr, file_id, record_id, data);
        // in case of success we expect message with the same length
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::write_file, request_data.size());
        createServerRequest(attr);
    }
}

void Client::readRecord(const std::uint16_t file_id, const std::uint16_t record_id, const std::uint16_t length)
{
    if (server_id != not_connected)
    {
        request_data = modbus_client.msgReadFileRecord(servers[server_id].info.addr, file_id, record_id, length);
        // amount of half words + 1 byte for ref type + 1 byte for data length
        // + 1 byte for resp length + 1 byte for func + modbus required part
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::read_file, static_cast<size_t>(modbus_client.getRequriedLength() + (length * 2) + 4));
        createServerRequest(attr);
    }
}

void Client::writeRegister(const std::uint16_t address, const std::uint16_t value)
{
    if (server_id != not_connected)
    {
        request_data = modbus_client.msgWriteRegister(servers[server_id].info.addr, address, value);
        // in case of success we expect message with the same length
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::write_register, request_data.size());
        createServerRequest(attr);
    }
}

void Client::readRegisters(const std::uint16_t address, const std::uint16_t quantity)
{
    if (server_id != not_connected)
    {
        request_data = modbus_client.msgReadRegisters(servers[server_id].info.addr, address, quantity);
        // amount of 16 bit registers + 1 byte for length + 1 byte for func +
        // modbus required part
        TaskAttributes attr =
            TaskAttributes(modbus::FunctionCodes::read_registers, static_cast<size_t>(modbus_client.getRequriedLength() + (quantity * 2) + 2));
        createServerRequest(attr);
    }
}

void Client::writeFile(const ServerFiles file_id)
{
    if (server_id == not_connected)
    {
        return;
    }
    std::uint8_t block_size = servers[server_id].regs[static_cast<int>(ServerRegisters::record_size)];
    std::uint16_t id = static_cast<std::uint16_t>(file_id);
    auto num_of_records = file.getNumOfRecords();
    task_info.reset(ClientTasks::file_write, num_of_records);
    for (auto i = 0; i < num_of_records; ++i)
    {
        std::vector<uint8_t> data;
        data.assign(&(file.getData()[i * block_size]), &(file.getData()[i * block_size]) + block_size);

        q_exchange.push([this, data, id, i] { writeRecord(id, static_cast<std::uint16_t>(i), data); });
    }
}

void Client::readFile(const ServerFiles file_id)
{
    if (server_id == not_connected)
    {
        return;
    }
    size_t file_size = 0;
    switch (file_id)
    {
        case ServerFiles::application:
            file_size = servers[server_id].regs[static_cast<int>(ServerRegisters::app_size)];
            break;

        case ServerFiles::server_metadata:
            file_size = sizeof(BootloaderInfo);
            break;
    }
    if (file.fileReadSetup(static_cast<std::uint16_t>(file_id),file_size, servers[server_id].regs[static_cast<int>(ServerRegisters::record_size)]))
    {
        auto num_of_records = file.getNumOfRecords();
        task_info.reset(ClientTasks::file_read, num_of_records);
        for (auto i = 0; i < num_of_records; ++i)
        {
            auto words_in_record = file.getActualRecordLength(i) / 2;
            q_exchange.push([this, words_in_record, file_id, i]
                            { readRecord(static_cast<std::uint16_t>(file_id), static_cast<std::uint16_t>(i), words_in_record); });
        }
    }
}

void Client::ping()
{
    if (server_id != not_connected)
    {
        std::uint8_t address = servers[server_id].info.addr;
        std::uint8_t function = static_cast<uint8_t>(modbus::FunctionCodes::undefined);
        std::vector<uint8_t> message{0x00, 0x00, 0x00, 0x00};
        request_data = modbus_client.msgCustom(address, function, message);
        // 1 byte for exception + 1 byte for func + modbus required part
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::undefined, static_cast<size_t>(modbus_client.getRequriedLength() + 2));
        createServerRequest(attr);
    }
}

void Client::exchangeCallback()
{
    auto readRegs = [](ServerData& server, const std::vector<uint8_t>& message)
    {
        const int id_length = 2;
        const int id_start = 3;
        int counter = 0;
        if (message[id_start] > (message.size() - (modbus::crc_size + modbus::address_size + modbus::function_size + 1)))
        {
            return;
        }

        for (int i = id_start; i < (id_start + message[id_length]); i = i + 2)
        {
            server.regs[counter] = static_cast<std::uint16_t>(message[i]) << 8;
            server.regs[counter] |= message[i + 1];
            ++counter;
        }
    };

    if (server_id == not_connected)
    {
        return;
    }
    ++task_info.counter;
    //std::printf("\n\r");
    //for(int i = 0; i < responce_data.size(); ++i)
    //{
    //    std::printf("0x%x ",responce_data[i]);
    //}
    //std::printf("\n\r");
    if (modbus_client.isChecksumValid(responce_data))
    {
        std::vector<std::uint8_t> message;
        modbus_client.extractData(responce_data, message);
        if (responce_data.size() != task_info.attributes.length)
        {
            task_info.error_code = make_error_code(ClientErrors::server_exception);
        }
        else
        {
            switch (task_info.task)
            {
                case ClientTasks::ping: //mark server as available if we have responce on this command
                    servers[server_id].info.status = ServerStatus::Available;
                    break;
                    
                case ClientTasks::regs_read:
                    readRegs(servers[server_id], message);
                    break;
                    
                case ClientTasks::file_read:
                    
                    fileReadCallback(message);
                    break;
                    
                case ClientTasks::file_write:
                    std::printf("progress: %d%% \n",getActualTaskProgress());
                    break;
                    
                default:
                    //nothing to do for now
                    break;
            }
        }

        if (task_info.counter == task_info.num_of_exchanges)
        {
            task_info.done = true;
        }
    }
    else
    {
        if (responce_data.size() == 0)
        {
            task_info.error_code = make_error_code(ClientErrors::timeout);
        }
        else
        {
            task_info.error_code = make_error_code(ClientErrors::bad_crc);
        }
        task_info.done = true;
    }
}

void Client::fileReadCallback(std::vector<std::uint8_t>& message)
{
    if (!file.getRecordFromMessage(message))
    {
        task_info.error_code = make_error_code(ClientErrors::internal);
    }
    else
    {
        if (file.isFileReady())
        {
            switch(file.getId())
            {
                case static_cast<std::uint16_t>(ServerFiles::server_metadata):
                    std::memcpy(&servers[server_id].data, &file.getData()[0], sizeof(BootloaderInfo));
                    break;
                
                default:
                    break;
            }
        }
    }
}

void Client::createServerRequest(const TaskAttributes& attr)
{
    task_info.attributes = attr;
    task = std::async(&Client::callServerExchange, this);
}

void Client::callServerExchange()
{
    responce_data.clear();
    try
    {
        serial_port.port.writeBinary(request_data);
    }
    catch (const std::system_error& e)
    {
        task_info.error_code = e.code();
    }
    //std::printf("data sent : size %d \n",request_data.size());
    // read from server
    try
    {
        serial_port.port.readBinary(responce_data, task_info.attributes.length);
    }
    catch (const std::system_error& e)
    {
        task_info.error_code = e.code();
    }
    //std::printf("data received, size : %d \n",responce_data.size());
}
} // namespace sm
