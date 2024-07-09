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

void Client::getServerData(const std::uint8_t address, ServerData& data)
{
    int index = getServerIndex(address);
    if (index != -1)
    {
        data = servers[index];
    }
    else
    {
        data = ServerData();
    }
}

int Client::getActualTaskProgress() const { return (task_info.counter * 100) / task_info.num_of_exchanges; }

void Client::addServer(const std::uint8_t addr, const std::uint8_t gateway_addr)
{
    if (getServerIndex(addr) == -1)
    {
        servers.push_back(ServerData());
        servers.back().info.addr = addr;
        servers.back().info.gateway_addr = gateway_addr;
    }
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

std::error_code Client::connect(const std::uint8_t address)
{
    // flush port buffer first
    serial_port.port.flushPort();
    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    
    // (1) ping server, expected answer with exception type 1
    task_info.error_code = taskPing(address);
    if (task_info.error_code)
    {
        return task_info.error_code;
    }
    
    // (2) load all registers
    task_info.error_code = taskReadRegisters(address, modbus::holding_regs_offset, amount_of_regs);
    
    if (task_info.error_code)
    {
        return task_info.error_code;
    }
    
     
    // (3.1) prepare to read
    task_info.error_code = taskWriteRegister(address, static_cast<std::uint16_t>(ServerRegisters::file_control), file_read_prepare);
    if (task_info.error_code)
    {
        return task_info.error_code;
    }
    
    // (3.2) file reading
    task_info.error_code = taskReadFile(address, ServerFiles::server_metadata);
    
    return task_info.error_code;
    
}

std::error_code Client::eraseApp(const std::uint8_t address)
{
    // flush port buffer first
    serial_port.port.flushPort();
    // (1) erase request
    // we may have here timeout problem because flash erase take a lot of time in some cases, add additional logic for this case in future
    task_info.error_code = taskWriteRegister(address, static_cast<std::uint16_t>(ServerRegisters::app_erase), app_erase_request);
    if (task_info.error_code)
    {
        return task_info.error_code;
    }
    // (2) read register with status information
    task_info.error_code = taskReadRegisters(address, modbus::holding_regs_offset, amount_of_regs);
    if (task_info.error_code)
    {
        return task_info.error_code;
    }
    return task_info.error_code;
}

std::error_code Client::uploadApp(const std::uint8_t address, const std::string path_to_file)
{
    // flush port buffer first
    serial_port.port.flushPort();
    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    int index = getServerIndex(address);
    if (index == -1)
    {
        return task_info.error_code;
    }
    std::uint8_t record_size = servers[index].regs[static_cast<std::uint8_t>(ServerRegisters::record_size)];
    if (file.fileExternalWriteSetup(static_cast<std::uint16_t>(ServerFiles::application), path_to_file, record_size))
    {

        // (1) erase application
        task_info.error_code = taskWriteRegister(address, static_cast<std::uint16_t>(ServerRegisters::app_erase), app_erase_request);
        if (task_info.error_code)
        {
            return task_info.error_code;
        }   
        // (2) send new file size
        std::uint16_t num_of_records = file.getNumOfRecords();
        
        task_info.error_code = taskWriteRegister(address, static_cast<std::uint16_t>(ServerRegisters::app_size), num_of_records);
        if (task_info.error_code)
        {
            return task_info.error_code;
        }
        // (3) prepare to write
        task_info.error_code = taskWriteRegister(address, static_cast<std::uint16_t>(ServerRegisters::file_control), file_write_prepare);
        if (task_info.error_code)
        {
            return task_info.error_code;
        }
        // (4) file sending
        task_info.error_code = taskWriteFile(address);
        if (task_info.error_code)
        {
            return task_info.error_code;
        }
        // (5) read status back
        task_info.error_code = taskReadRegisters(address, modbus::holding_regs_offset, amount_of_regs);
    }
    else
    {
        task_info.error_code = make_error_code(ClientErrors::internal);
    }
    return task_info.error_code;
}

std::error_code Client::startApp(const std::uint8_t address)
{
    // flush port buffer first
    serial_port.port.flushPort();
    task_info.error_code = taskWriteRegister(address, static_cast<std::uint16_t>(ServerRegisters::app_start), app_start_request);
    return task_info.error_code;
}

std::error_code Client::taskPing(const std::uint8_t dev_addr)
{
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
    int index = getServerIndex(dev_addr);
    if (index == -1)
    {
        return task_info.error_code;
    }
    // we are trying to reach this server through the gateway, perform gateway setup first
    if (servers[index].info.gateway_addr != 0)
    {
        std::uint16_t expected_length = modbus_client.getRequriedLength() + 2;
        std::uint16_t control_reg = static_cast<std::uint16_t>(ServerRegisters::gateway_buffer_size);
        auto error = taskWriteRegister(servers[index].info.gateway_addr, control_reg, expected_length);
        if (error)
        {
            task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
            return task_info.error_code;
        }
    }
    // direct address ping
    task_info.reset(ClientTasks::ping, 1, index);
    q_task.push([this, lambda_ping, dev_addr]() { q_exchange.push([lambda_ping, dev_addr] { lambda_ping(dev_addr); }); });
    while (!task_info.done)
        ;
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
    int index = getServerIndex(dev_addr);
    if (index == -1)
    {
        return task_info.error_code;
    }
    // we are trying to reach this server through the gateway, perform gateway setup first
    if ((servers[index].info.gateway_addr != 0) && !recurced)
    {
        recurced = true;
        std::uint16_t expected_length = modbus_client.getRequriedLength() + 5;
        std::uint16_t control_reg = static_cast<std::uint16_t>(ServerRegisters::gateway_buffer_size);
        auto error = taskWriteRegister(servers[index].info.gateway_addr, control_reg, expected_length);
        if (error)
        {
            task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
            recurced = false;
            return task_info.error_code;
        }
    }

    // direct register write
    task_info.reset(ClientTasks::reg_write, 1,index);
    q_task.push([this, lambda_write_reg, dev_addr, reg_addr, value]()
                { q_exchange.push([lambda_write_reg, dev_addr, reg_addr, value] { lambda_write_reg(dev_addr, reg_addr, value); }); });
    while (!task_info.done)
        ;
    recurced = false;
    return task_info.error_code;
}

std::error_code Client::taskReadRegisters(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t quantity)
{
    auto lambda_read_regs = [this](const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t quantity)
    {
        request_data = modbus_client.msgReadRegisters(dev_addr, reg_addr, quantity);
        // amount of 16 bit registers + 1 byte for length + 1 byte for func + modbus required part
        size_t expected_length = static_cast<size_t>(modbus_client.getRequriedLength() + (quantity * 2) + 2);
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::read_registers, expected_length);
        createServerRequest(attr);
    };
    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    int index = getServerIndex(dev_addr);
    if (index == -1)
    {
        return task_info.error_code;
    }
    // we are trying to reach this server through the gateway, perform gateway setup first
    if (servers[index].info.gateway_addr != 0)
    {
        std::uint16_t expected_length = static_cast<size_t>(modbus_client.getRequriedLength() + (quantity * 2) + 2);
        std::uint16_t control_reg = static_cast<std::uint16_t>(ServerRegisters::gateway_buffer_size);
        auto error = taskWriteRegister(servers[index].info.gateway_addr, control_reg, expected_length);
        if (error)
        {
            task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
            return task_info.error_code;
        }
    }
    // direct registers reading
    task_info.reset(ClientTasks::regs_read, 1, index);
    q_task.push([this, lambda_read_regs, dev_addr, reg_addr, quantity]()
                { q_exchange.push([lambda_read_regs, dev_addr, reg_addr, quantity] { lambda_read_regs(dev_addr, reg_addr, quantity); }); });
    while (!task_info.done)
        ;
    return task_info.error_code;
}

std::error_code Client::taskReadFile(const std::uint8_t dev_addr, const ServerFiles file_id)
{
    auto lambda_read_record = [this](const std::uint8_t dev_addr, const std::uint16_t file_id, const std::uint16_t record_id, const std::uint16_t length)
    {
        request_data = modbus_client.msgReadFileRecord(dev_addr, file_id, record_id, length);
        // amount of half words + 1 byte for ref type + 1 byte for data length
        // + 1 byte for resp length + 1 byte for func + modbus required part
        size_t expected_length = static_cast<size_t>(modbus_client.getRequriedLength() + (length * 2) + 4);
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::read_file, expected_length);
        createServerRequest(attr);
    };

    auto lambda_read_file =
        [this, lambda_read_record](const std::uint8_t dev_addr,const int index, const std::uint16_t file_id)
    {
        auto num_of_records = file.getNumOfRecords();
        task_info.reset(ClientTasks::file_read, num_of_records, index);
        for (auto i = 0; i < num_of_records; ++i)
        {
            auto words_in_record = file.getActualRecordLength(i) / 2;
            q_exchange.push([words_in_record, file_id, i, lambda_read_record, dev_addr]
                            { lambda_read_record(dev_addr, file_id, static_cast<std::uint16_t>(i), words_in_record); });
        }
    };

    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    int index = getServerIndex(dev_addr);
    if (index == -1)
    {
        return task_info.error_code;
    }

    auto record_size = servers[index].regs[static_cast<int>(ServerRegisters::record_size)];
    auto converted_file_id = static_cast<std::uint16_t>(file_id);
    auto file_size = getFileSize(file_id);
    if(file.fileReadSetup(converted_file_id, file_size, record_size) != true)
    {
        task_info.error_code = make_error_code(ClientErrors::server_not_connected);
        return task_info.error_code;
    }

    // we are trying to reach this server through the gateway, perform gateway setup first
    if (servers[index].info.gateway_addr != 0)
    {
        std::uint16_t expected_length = static_cast<size_t>(modbus_client.getRequriedLength() + record_size + 4);
        std::uint16_t control_reg = static_cast<std::uint16_t>(ServerRegisters::gateway_buffer_size);
        auto error = taskWriteRegister(servers[index].info.gateway_addr, control_reg, expected_length);
        if (error)
        {
            task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
            return task_info.error_code;
        }
        control_reg = static_cast<std::uint16_t>(ServerRegisters::record_counter);
        error = taskWriteRegister(servers[index].info.gateway_addr, control_reg, file.getNumOfRecords());
        if (error)
        {
            task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
            return task_info.error_code;
        }

        control_reg = static_cast<std::uint16_t>(ServerRegisters::gateway_file_control);
        error = taskWriteRegister(servers[index].info.gateway_addr, control_reg, file_read_prepare);
        if (error)
        {
            task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
            return task_info.error_code;
        }
    }
    task_info.reset();
    q_task.push([this, dev_addr,index, lambda_read_file, converted_file_id]()
                { lambda_read_file(dev_addr,index, converted_file_id); });
    while (!task_info.done)
        ;
    return task_info.error_code;
}

std::error_code Client::taskWriteFile(const std::uint8_t dev_addr)
{
    auto lambda_write_record =
        [this](const std::uint8_t dev_addr, const std::uint16_t file_id, const std::uint16_t record_id, const std::vector<std::uint8_t>& data)
    {
        request_data = modbus_client.msgWriteFileRecord(dev_addr, file_id, record_id, data);
        // in case of success we expect message with the same length
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::write_file, request_data.size());
        createServerRequest(attr);
    };

    auto lambda_write_file = [this, lambda_write_record](const std::uint8_t dev_addr,const int index, const std::uint16_t record_size)
    {
        const std::uint16_t num_of_records = file.getNumOfRecords();
        const std::uint16_t file_id = file.getId();
        task_info.reset(ClientTasks::file_write, num_of_records, index);
        for (auto i = 0; i < num_of_records; ++i)
        {
            std::vector<uint8_t> data;
            data.assign(&(file.getData()[i * record_size]), &(file.getData()[i * record_size]) + record_size);
            q_exchange.push([this, lambda_write_record, dev_addr, file_id, i, data]
                            { lambda_write_record(dev_addr, file_id, static_cast<std::uint16_t>(i), data); });
        }
    };

    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    int index = getServerIndex(dev_addr);
    if (index == -1)
    {
        return task_info.error_code;
    }
    auto record_size = servers[index].regs[static_cast<int>(ServerRegisters::record_size)];
    // we are trying to reach this server through the gateway, perform gateway setup first
    if (servers[index].info.gateway_addr != 0)
    {
        std::uint16_t expected_length = static_cast<size_t>(modbus_client.getRequriedLength() + record_size + 9);
        std::uint16_t control_reg = static_cast<std::uint16_t>(ServerRegisters::gateway_buffer_size);
        auto error = taskWriteRegister(servers[index].info.gateway_addr, control_reg, expected_length);
        if (error)
        {
            task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
            return task_info.error_code;
        }
        control_reg = static_cast<std::uint16_t>(ServerRegisters::record_counter);
        error = taskWriteRegister(servers[index].info.gateway_addr, control_reg, file.getNumOfRecords());
        if (error)
        {
            task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
            return task_info.error_code;
        }

        control_reg = static_cast<std::uint16_t>(ServerRegisters::gateway_file_control);
        error = taskWriteRegister(servers[index].info.gateway_addr, control_reg, file_write_prepare);
        if (error)
        {
            task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
            return task_info.error_code;
        }
    }
    task_info.reset();
    q_task.push([this, dev_addr, lambda_write_file, index, record_size]() { lambda_write_file(dev_addr, index, record_size); });
    while (!task_info.done)
        ;
    return task_info.error_code;
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
                if (!error_in_task)
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
    ++task_info.counter;
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
                case ClientTasks::ping: // mark server as available if we have responce on this command
                    servers[task_info.index].info.status = ServerStatus::Available;
                    break;

                case ClientTasks::regs_read:
                    readRegs(servers[task_info.index], message);
                    break;

                case ClientTasks::file_read:

                    fileReadCallback(message,task_info.index);
                    break;

                case ClientTasks::file_write:
                    printProgressBar(getActualTaskProgress());
                    //std::printf("progress: %d%% \n", getActualTaskProgress());
                    break;

                default:
                    // nothing to do for now
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

void Client::fileReadCallback(std::vector<std::uint8_t>& message, const int index)
{
    if (!file.getRecordFromMessage(message))
    {
        task_info.error_code = make_error_code(ClientErrors::internal);
    }
    else
    {
        if (file.isFileReady())
        {
            switch (file.getId())
            {
                case static_cast<std::uint16_t>(ServerFiles::server_metadata):
                    std::memcpy(&servers[index].data, &file.getData()[0], sizeof(BootloaderInfo));
                    break;

                default:
                    break;
            }
        }
    }
}

size_t Client::getFileSize(const ServerFiles file_id)
{
    size_t file_size = 0;
    switch (file_id)
    {
        case ServerFiles::server_metadata:
            file_size = sizeof(BootloaderInfo);
            break;
        case ServerFiles::application:
        default:
            break;
    }
    return file_size;
}

void Client::printProgressBar(const int task_progress)
{
    float progress = 0.01 * task_progress;
    int barWidth = 80;
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) 
    {
        if (i < pos)
        {
            std::cout << "*";
        }
        else if (i == pos) 
        {
            std::cout << ")";
        }
        else
        {
            std::cout << " ";
        }
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
    if(task_progress == 100)
    {
        std::cout<<std::endl;
    }
}

int Client::getServerIndex(const std::uint8_t address)
{
    auto it = std::find_if(servers.begin(), servers.end(), [address](ServerData& server) { return server.info.addr == address; });
    if (it != servers.end())
    {
        return std::distance(servers.begin(), it);
    }
    else
    {
        return -1;
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
    //std::printf("******************************************\n");
    //std::printf("data sent : size %d \n",request_data.size());
    //for(int i = 0; i < request_data.size(); ++i)
    //{
    //    std::printf("0x%x ",request_data[i]);
    //}
    //std::printf("\n\r");
    try
    {
        serial_port.port.readBinary(responce_data, task_info.attributes.length);
    }
    catch (const std::system_error& e)
    {
        task_info.error_code = e.code();
    }
    //std::printf("data received, size : %d \n",responce_data.size());
    //for(int i = 0; i < responce_data.size(); ++i)
    //{
    //    std::printf("0x%x ",responce_data[i]);
    //}
    //std::printf("\n\r");
    //std::printf("******************************************\n");
    //std::printf("\n\r");
}
} // namespace sm
