/**
 * @file sm_client.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>

#include "../../common/sm_modbus.hpp"
#include "../inc/sm_client.hpp"
#include "../inc/sm_error.hpp"

namespace sm
{

ModbusClient::~ModbusClient()
{
    thread_stop.store(true, std::memory_order_relaxed);
    client_thread.join();
}

int ModbusClient::getActualTaskProgress() const { return (task_info.counter * 100) / task_info.num_of_exchanges; }

void ModbusClient::getLastServerRegList(const std::uint8_t addr, std::vector<std::uint16_t>& registers)
{
    registers.clear();
    auto index = getServerIndex(addr);
    if ( index != -1)
    {
        registers.insert(registers.end(),servers[index].regs.begin(),servers[index].regs.end());
    }
}

void ModbusClient::addServer(const std::uint8_t addr, const std::uint8_t gateway_addr)
{
    if (getServerIndex(addr) == -1)
    {
        servers.push_back(ServerData());
        servers.back().info.addr = addr;
        servers.back().info.gateway_addr = gateway_addr;
    }
}

std::error_code ModbusClient::start(std::string device)
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

void ModbusClient::stop()
{
    if (serial_port.getState() == sp::PortState::Open)
    {
        serial_port.close();
    }
}

std::error_code ModbusClient::configure(sp::PortConfig config)
{
    task_info.error_code = serial_port.setup(config);
    return task_info.error_code;
}

std::error_code ModbusClient::taskPing(const std::uint8_t dev_addr)
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

std::error_code ModbusClient::taskWriteRegister(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t value)
{
    static bool recurced = false;
    auto lambda_write_reg = [this](const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t value)
    {
        request_data = modbus_client.msgWriteRegister(dev_addr, reg_addr, value);
        // in case of success we expect message with the same length
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::write_reg, request_data.size());
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
    task_info.reset(ClientTasks::reg_write, 1, index);
    q_task.push([this, lambda_write_reg, dev_addr, reg_addr, value]()
                { q_exchange.push([lambda_write_reg, dev_addr, reg_addr, value] { lambda_write_reg(dev_addr, reg_addr, value); }); });
    while (!task_info.done)
        ;
    recurced = false;
    return task_info.error_code;
}

std::error_code ModbusClient::taskReadRegisters(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t quantity)
{
    auto lambda_read_regs = [this](const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t quantity)
    {
        request_data = modbus_client.msgReadRegisters(dev_addr, reg_addr, quantity);
        // amount of 16 bit registers + 1 byte for length + 1 byte for func + modbus required part
        size_t expected_length = static_cast<size_t>(modbus_client.getRequriedLength() + (quantity * 2) + 2);
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::read_regs, expected_length);
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

std::error_code ModbusClient::taskReadFile(const std::uint8_t dev_addr, const std::uint16_t file_id, const std::size_t file_size)
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

    auto lambda_read_file = [this, lambda_read_record](const std::uint8_t dev_addr, const int index, const std::uint16_t file_id)
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
    if (file.fileReadSetup(file_id, file_size, record_size) != true)
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

        control_reg = static_cast<std::uint16_t>(ServerRegisters::file_control);
        error = taskWriteRegister(servers[index].info.gateway_addr, control_reg, file_read_prepare);
        if (error)
        {
            task_info.error_code = make_error_code(ClientErrors::gateway_not_responding);
            return task_info.error_code;
        }
    }
    task_info.reset();
    q_task.push([this, dev_addr, index, lambda_read_file, file_id]() { lambda_read_file(dev_addr, index, file_id); });
    while (!task_info.done)
        ;
    return task_info.error_code;
}

std::error_code ModbusClient::taskWriteFile(const std::uint8_t dev_addr)
{
    auto lambda_write_record =
        [this](const std::uint8_t dev_addr, const std::uint16_t file_id, const std::uint16_t record_id, const std::vector<std::uint8_t>& data)
    {
        request_data = modbus_client.msgWriteFileRecord(dev_addr, file_id, record_id, data);
        // in case of success we expect message with the same length
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::write_file, request_data.size());
        createServerRequest(attr);
    };

    auto lambda_write_file = [this, lambda_write_record](const std::uint8_t dev_addr, const int index, const std::uint16_t record_size)
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

        control_reg = static_cast<std::uint16_t>(ServerRegisters::file_control);
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

void ModbusClient::clientThread()
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

void ModbusClient::exchangeCallback()
{
    auto readRegs = [](ServerData& server, const std::vector<uint8_t>& message)
    {
        server.regs.clear();
        const int id_length = 2;
        const int id_start = 3;
        int counter = 0;
        if (message[id_start] > (message.size() - (modbus::crc_size + modbus::address_size + modbus::function_size + 1)))
        {
            return;
        }

        for (int i = id_start; i < (id_start + message[id_length]); i = i + 2)
        {
            std::uint16_t reg = static_cast<std::uint16_t>(message[i]) << 8;
            reg |= message[i + 1];
            server.regs.push_back(reg);
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
                    fileReadCallback(message);
                    break;

                case ClientTasks::file_write:
                    printProgressBar(getActualTaskProgress());
                    // std::printf("progress: %d%% \n", getActualTaskProgress());
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

void ModbusClient::fileReadCallback(std::vector<std::uint8_t>& message)
{
    if (!file.getRecordFromMessage(message))
    {
        task_info.error_code = make_error_code(ClientErrors::internal);
    }
}

void ModbusClient::printProgressBar(const int task_progress)
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
    if (task_progress == 100)
    {
        std::cout << std::endl;
    }
}

int ModbusClient::getServerIndex(const std::uint8_t address)
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

void ModbusClient::createServerRequest(const TaskAttributes& attr)
{
    task_info.attributes = attr;
    task = std::async(&ModbusClient::callServerExchange, this);
}

void ModbusClient::callServerExchange()
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
    try
    {
        serial_port.port.readBinary(responce_data, task_info.attributes.length);
    }
    catch (const std::system_error& e)
    {
        task_info.error_code = e.code();
    }
}
} // namespace sm
