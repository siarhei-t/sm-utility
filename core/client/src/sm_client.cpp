/**
 * @file sm_client.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "../../common/sm_common.hpp"
#include "../inc/sm_client.hpp"
#include "../inc/sm_error.hpp"

namespace sm
{

int ModbusClient::getActualTaskProgress() const 
{
    auto volume = task_info.num_of_exchanges;
    auto progress = task_info.counter;
    if((volume > 0) && (progress <= volume))
    {
        return (progress * task_complete_value) / volume;
    }
    else
    {
        return task_not_started_value;
    }
}

int ModbusClient::getServerIndex(const std::uint8_t address) const
{
    auto it = std::find_if(servers.begin(), servers.end(), [address](const ServerData& server) { return server.info.addr == address; });
    if (it != servers.end())
    {
        return std::distance(servers.begin(), it);
    }
    else
    {
        return server_not_found;
    }
}

void ModbusClient::getLastServerRegList(const std::uint8_t dev_addr, ServerRegisters& registers)
{
    registers = ServerRegisters();
    auto index = getServerIndex(dev_addr);
    if (index != server_not_found)
    {
        registers = servers[index].registers;
    }
}

bool ModbusClient::setServerAsAvailable(const std::uint8_t dev_addr)
{
    auto index = getServerIndex(dev_addr);
    if (index != server_not_found)
    {
        servers[index].info.status = ServerStatus::available;
        return true;
    }
    else
    {
        return false;
    }
}

bool ModbusClient::setServerRecordMaxSize(const std::uint8_t dev_addr, const std::uint8_t record_size)
{
    auto index = getServerIndex(dev_addr);
    if (index != server_not_found)
    {
        servers[index].info.record_size = record_size;
        return true;
    }
    else
    {
        return false;
    }
}

void ModbusClient::printProgressBar(const int task_progress)
{
    float progress = 0.01 * task_progress;
    int bar_width = 80;
    std::cout << "[";
    int pos = bar_width * progress;
    for (int i = 0; i < bar_width; ++i) 
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
    std::cout << "] " << int(progress * task_complete_value) << " %\r";
    std::cout.flush();
    if(task_progress == task_complete_value)
    {
        std::cout<<std::endl;
    }
}

void ModbusClient::addServer(const std::uint8_t addr, const std::uint8_t gateway_addr)
{
    if (getServerIndex(addr) == server_not_found)
    {
        servers.push_back(ServerData());
        servers.back().info.addr = addr;
        servers.back().info.gateway_addr = gateway_addr;
    }
}

std::error_code ModbusClient::start(std::string device)
{
    auto error_code = std::error_code();
    if (serial_port.getState() != sp::PortState::Open)
    {
        error_code = serial_port.open(device);
    }
    else
    {
        if (device != serial_port.getPath())
        {
            serial_port.close();
            error_code = serial_port.open(device);
        }
    }
    return error_code;
}

void ModbusClient::stop()
{
    if (serial_port.getState() == sp::PortState::Open)
    {
        serial_port.close();
    }
    task_info.reset();
}

std::error_code ModbusClient::configure(sp::PortConfig config)
{
    return serial_port.setup(config);
}

std::error_code ModbusClient::taskPing(const std::uint8_t dev_addr)
{
    auto lambda_ping = [this](const std::uint8_t address)
    {
        std::uint8_t function = static_cast<uint8_t>(modbus::FunctionCodes::undefined);
        std::vector<std::uint8_t> message{0x00, 0x00, 0x00, 0x00};
        modbus_message.msgCustom(request_data, function, message, address);
        // 1 byte for exception + 1 byte for func + modbus required part
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::undefined, getExpectedLength(ClientTasks::ping));
        createServerRequest(attr);
    };

    int index = getServerIndex(dev_addr);
    if (index == server_not_found)
    {
        return make_error_code(ClientErrors::server_not_connected);
    }
    // we are trying to reach this server through the gateway, perform gateway setup first
    if (servers[index].info.gateway_addr != 0)
    {
        auto gateway_index = getServerIndex(servers[index].info.gateway_addr);
        if(servers[gateway_index].info.status == ServerStatus::unavailable)
        {
            return make_error_code(ClientErrors::gateway_not_connected);
        }
        std::uint16_t expected_length = getExpectedLength(ClientTasks::ping);
        auto error_code = taskWriteRegister(servers[index].info.gateway_addr, RegisterDefinitions::gateway_buffer_size, expected_length);
        if (error_code)
        {
            return error_code;
        }
    }
    task_info.reset(ClientTasks::ping, 1, index);
    q_task.push([this, lambda_ping, dev_addr]() { q_exchange.push([lambda_ping, dev_addr] { lambda_ping(dev_addr); }); });
    while (!task_info.done.load(std::memory_order_relaxed))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(default_task_wait_delay_ms));
    }
    return task_info.error_code;
}

std::error_code ModbusClient::taskWriteRegister(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t value, const bool print_progress)
{
    static bool recursed = false;
    auto lambda_write_reg = [this](const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t value)
    {
        modbus_message.msgWriteRegister(request_data, reg_addr, value, dev_addr);
        // in case of success we expect message with the same length
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::write_reg, getExpectedLength(ClientTasks::reg_write));
        createServerRequest(attr);
    };
    int index = getServerIndex(dev_addr);
    if (index == server_not_found)
    {
        return make_error_code(ClientErrors::server_not_connected);
    }
    if (servers[index].info.status == ServerStatus::unavailable)
    {
        return make_error_code(ClientErrors::server_not_connected);
    }
    // we are trying to reach this server through the gateway, perform gateway setup first
    if ((servers[index].info.gateway_addr != 0) && !recursed)
    {
        recursed = true;
        auto gateway_index = getServerIndex(servers[index].info.gateway_addr);
        if(servers[gateway_index].info.status == ServerStatus::unavailable)
        {
            recursed = false;
            return make_error_code(ClientErrors::gateway_not_connected);
        }
        std::uint16_t expected_length = getExpectedLength(ClientTasks::reg_write);
        auto error_code = taskWriteRegister(servers[index].info.gateway_addr, RegisterDefinitions::gateway_buffer_size, expected_length);
        if (error_code)
        {
            recursed = false;
            return error_code;
        }
    }
    task_info.reset(ClientTasks::reg_write, 1, index, print_progress);
    q_task.push([this, lambda_write_reg, dev_addr, reg_addr, value]()
                { q_exchange.push([lambda_write_reg, dev_addr, reg_addr, value] { lambda_write_reg(dev_addr, reg_addr, value); }); });
    while (!task_info.done.load(std::memory_order_relaxed))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(default_task_wait_delay_ms));
    }
    recursed = false;
    return task_info.error_code;
}

std::error_code ModbusClient::taskReadRegisters(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t quantity, const bool print_progress)
{
    auto lambda_read_regs = [this](const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t quantity)
    {
        modbus_message.msgReadRegisters(request_data, reg_addr, quantity, dev_addr);
        // amount of 16 bit registers + 1 byte for length + 1 byte for func + modbus required part
        size_t expected_length = getExpectedLength(ClientTasks::regs_read, quantity * 2);
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::read_regs, expected_length);
        createServerRequest(attr);
    };
    int index = getServerIndex(dev_addr);
    if (index == server_not_found)
    {
        return make_error_code(ClientErrors::server_not_connected);
    }
    if (servers[index].info.status == ServerStatus::unavailable)
    {
        return make_error_code(ClientErrors::server_not_connected);
    }
    // we are trying to reach this server through the gateway, perform gateway setup first
    if (servers[index].info.gateway_addr != 0)
    {
        auto gateway_index = getServerIndex(servers[index].info.gateway_addr);
        if(servers[gateway_index].info.status == ServerStatus::unavailable)
        {
            return make_error_code(ClientErrors::gateway_not_connected);
        }
        size_t expected_length = getExpectedLength(ClientTasks::regs_read, quantity * 2);
        auto error_code = taskWriteRegister(servers[index].info.gateway_addr, RegisterDefinitions::gateway_buffer_size, expected_length);
        if (error_code)
        {
            return error_code;
        }
    }
    task_info.reset(ClientTasks::regs_read, 1, index,print_progress);
    servers[index].registers.reg_start_address = reg_addr;
    servers[index].registers.values.clear();
    q_task.push([this, lambda_read_regs, dev_addr, reg_addr, quantity]()
                { q_exchange.push([lambda_read_regs, dev_addr, reg_addr, quantity] { lambda_read_regs(dev_addr, reg_addr, quantity); }); });
    while (!task_info.done.load(std::memory_order_relaxed))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(default_task_wait_delay_ms));
    }
    return task_info.error_code;
}

std::error_code ModbusClient::taskReadFile(const std::uint8_t dev_addr, const std::uint16_t file_id, const std::size_t file_size, const bool print_progress)
{
    auto lambda_read_record = [this](const std::uint8_t dev_addr, const std::uint16_t file_id, const std::uint16_t record_id, const std::uint16_t length)
    {
        modbus_message.msgReadFileRecord(request_data, file_id, record_id, length, dev_addr);
        // amount of half words + 1 byte for ref type + 1 byte for data length
        // + 1 byte for resp length + 1 byte for func + modbus required part
        size_t expected_length = getExpectedLength(ClientTasks::file_read, length * 2);
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::read_file, expected_length);
        createServerRequest(attr);
    };

    auto lambda_read_file = [this, lambda_read_record, print_progress](const std::uint8_t dev_addr, const int index, const std::uint16_t file_id)
    {
        const std::uint16_t num_of_records = file.getNumOfRecords();
        task_info.reset(ClientTasks::file_read, num_of_records, index, print_progress);
        for (std::uint16_t i = 0; i < num_of_records; ++i)
        {
            auto words_in_record = file.getActualRecordLength(i) / 2;
            q_exchange.push([words_in_record, file_id, i, lambda_read_record, dev_addr]
                            { lambda_read_record(dev_addr, file_id, i, words_in_record); });
        }
    };
    int index = getServerIndex(dev_addr);
    if (index ==server_not_found)
    {
        return make_error_code(ClientErrors::server_not_connected);
    }
    if (servers[index].info.status == ServerStatus::unavailable)
    {
        return make_error_code(ClientErrors::server_not_connected);
    }
    auto record_size = servers[index].info.record_size;
    if(record_size == 0)
    {
        return make_error_code(ClientErrors::max_record_length_not_configured);
    }
    if (file.fileReadSetup(file_id, file_size, record_size) != true)
    {
        return make_error_code(ClientErrors::internal);
    }
    auto error_code = taskWriteRegister(dev_addr, RegisterDefinitions::record_counter, file.getNumOfRecords());
    if (error_code)
    {
        return error_code;
    }
    error_code = taskWriteRegister(dev_addr, RegisterDefinitions::file_control, file_read_prepare);
    if (error_code)
    {
        return error_code;
    }
    // we are trying to reach this server through the gateway, perform gateway setup first
    if (servers[index].info.gateway_addr != 0)
    {
        auto gateway_index = getServerIndex(servers[index].info.gateway_addr);
        if(servers[gateway_index].info.status == ServerStatus::unavailable)
        {
            return make_error_code(ClientErrors::gateway_not_connected);
        }
        std::uint16_t expected_length = getExpectedLength(ClientTasks::file_read, record_size);
        error_code = taskWriteRegister(servers[index].info.gateway_addr, RegisterDefinitions::gateway_buffer_size, expected_length);
        if (error_code)
        {
            return error_code;
        }
        error_code = taskWriteRegister(servers[index].info.gateway_addr, RegisterDefinitions::record_counter, file.getNumOfRecords());
        if (error_code)
        {
            return error_code;
        }
        error_code = taskWriteRegister(servers[index].info.gateway_addr, RegisterDefinitions::file_control, file_read_prepare);
        if (error_code)
        {
            return error_code;
        }
    }
    task_info.reset();
    q_task.push([dev_addr, index, lambda_read_file, file_id]() { lambda_read_file(dev_addr, index, file_id); });
    while (!task_info.done.load(std::memory_order_relaxed))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(default_task_wait_delay_ms));
    }
    return task_info.error_code;
}

std::error_code ModbusClient::taskWriteFile(const std::uint8_t dev_addr, const bool print_progress)
{
    auto lambda_write_record =
        [this](const std::uint8_t dev_addr, const std::uint16_t file_id, const std::uint16_t record_id, const std::vector<std::uint8_t>& data)
    {
        modbus_message.msgWriteFileRecord(request_data, file_id, record_id, data, dev_addr);
        // in case of success we expect message with the same length
        std::uint16_t expected_length = getExpectedLength(ClientTasks::file_write, data.size());
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::write_file, expected_length);
        createServerRequest(attr);
    };

    auto lambda_write_file = [this, lambda_write_record, print_progress](const std::uint8_t dev_addr, const int index, const std::uint16_t record_size)
    {
        const std::uint16_t num_of_records = file.getNumOfRecords();
        const std::uint16_t file_id = file.getId();
        task_info.reset(ClientTasks::file_write, num_of_records, index, print_progress);
        for (std::uint16_t i = 0; i < num_of_records; ++i)
        {
            std::vector<std::uint8_t> data;
            data.assign(&(file.getData()[i * record_size]), &(file.getData()[i * record_size]) + record_size);
            q_exchange.push([lambda_write_record, dev_addr, file_id, i, data] { lambda_write_record(dev_addr, file_id, i, data); });
        }
    };
    int index = getServerIndex(dev_addr);
    if (index == server_not_found)
    {
        return make_error_code(ClientErrors::server_not_connected);
    }
    if (servers[index].info.status == ServerStatus::unavailable)
    {
        return make_error_code(ClientErrors::server_not_connected);
    }
    if (!file.isFileReady())
    {
        return make_error_code(ClientErrors::file_buffer_is_empty);
    }
    auto record_size = servers[index].info.record_size;
    if (record_size == 0)
    {
        return make_error_code(ClientErrors::max_record_length_not_configured);
    }
    auto error_code = taskWriteRegister(dev_addr, RegisterDefinitions::record_counter, file.getNumOfRecords());
    if (error_code)
    {
        return error_code;
    }
    error_code = taskWriteRegister(dev_addr, RegisterDefinitions::file_control, file_write_prepare);
    if (error_code)
    {
        return error_code;
    }
    // we are trying to reach this server through the gateway, perform gateway setup first
    if (servers[index].info.gateway_addr != 0)
    {
        auto gateway_index = getServerIndex(servers[index].info.gateway_addr);
        if(servers[gateway_index].info.status == ServerStatus::unavailable)
        {
            return make_error_code(ClientErrors::gateway_not_connected);
        }
        std::uint16_t expected_length = getExpectedLength(ClientTasks::file_write,record_size);
        error_code = taskWriteRegister(servers[index].info.gateway_addr, RegisterDefinitions::gateway_buffer_size, expected_length);
        if (error_code)
        {
            return error_code;
        }
        error_code = taskWriteRegister(servers[index].info.gateway_addr, RegisterDefinitions::record_counter, file.getNumOfRecords());
        if (error_code)
        {
            return error_code;
        }
        error_code = taskWriteRegister(servers[index].info.gateway_addr, RegisterDefinitions::file_control, file_write_prepare);
        if (error_code)
        {
            return error_code;
        }
    }
    task_info.reset();
    q_task.push([dev_addr, lambda_write_file, index, record_size]() { lambda_write_file(dev_addr, index, record_size); });
    while (!task_info.done.load(std::memory_order_relaxed))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(default_task_wait_delay_ms));
    }
    return task_info.error_code;
}

void ModbusClient::clientThread()
{
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
        std::this_thread::sleep_for(std::chrono::milliseconds(default_task_wait_delay_ms));
    }
}

void ModbusClient::exchangeCallback()
{
    auto readRegs = [](ServerData& server, const std::vector<uint8_t>& message)
    {
        server.registers.values.clear();
        const int id_length = modbus::read_regs_response_data_length_idx;
        if (message[id_length] > (message.size() - modbus::function_size - 1))
        {
            return;
        }
        std::uint16_t reg = 0;
        std::uint8_t num_of_bytes = message[id_length];
        int index = modbus::read_regs_response_data_start_idx;
        for (int i = 0; i < num_of_bytes / 2; ++i)
        {
            reg = static_cast<std::uint16_t>(message[index]) << 8;
            reg |= message[index + 1];
            server.registers.values.push_back(reg);
            index += 2;
        }
        auto amount_of_regs = server.registers.values.size();
        if(server.registers.reg_start_address <= (modbus::holding_regs_offset + RegisterDefinitions::record_size) &&
           (server.registers.reg_start_address + amount_of_regs) >= (modbus::holding_regs_offset + RegisterDefinitions::record_size)
          )
        {
            server.info.record_size = server.registers.values[RegisterDefinitions::record_size];
        }
    };

    ++task_info.counter;
    if (modbus_message.isChecksumValid(response_data))
    {
        std::vector<std::uint8_t> message;

        if ((response_data.size() != task_info.attributes.length) || !modbus_message.extractData(response_data, message))
        {
            task_info.error_code = make_error_code(ClientErrors::server_exception);
        }
        else
        {
            switch (task_info.task)
            {
                case ClientTasks::ping: // mark server as available if we have response on this command
                    servers[task_info.index].info.status = ServerStatus::available;
                    break;

                case ClientTasks::regs_read:
                    readRegs(servers[task_info.index], message);
                    break;

                case ClientTasks::file_read:
                    fileReadCallback(message);
                    break;

                default:
                    // nothing to do for now
                    break;
            }
            if(task_info.is_printable)
            {
                printProgressBar(getActualTaskProgress());
            }
        }
        if (task_info.counter == task_info.num_of_exchanges)
        {
            task_info.done.store(true, std::memory_order_relaxed);
        }
    }
    else
    {
        if (response_data.size() == 0)
        {
            servers[task_info.index].info.status = ServerStatus::unavailable;
            task_info.error_code = make_error_code(ClientErrors::timeout);
        }
        else
        {
            task_info.error_code = make_error_code(ClientErrors::bad_crc);
        }
        task_info.done.store(true, std::memory_order_relaxed);
    }
}

size_t ModbusClient::getExpectedLength(const ClientTasks task, const size_t extra) const
{
    switch (task)
    {
        case sm::ClientTasks::undefined:
            return 0;
        case sm::ClientTasks::ping:
            return modbus_message.getRequiredLength() + modbus::exception_pdu_size;

        case sm::ClientTasks::reg_write:
            return modbus_message.getRequiredLength() + modbus::response_write_reg_pdu_size;

        case sm::ClientTasks::regs_read:
            return modbus_message.getRequiredLength() + modbus::response_read_reg_pdu_part + extra;

        case sm::ClientTasks::file_write:
            return modbus_message.getRequiredLength() + modbus::response_write_file_pdu_part + extra;

        case sm::ClientTasks::file_read:
            return modbus_message.getRequiredLength() + modbus::response_read_file_pdu_part + extra;
    };
    return 0;
}

void ModbusClient::fileReadCallback(std::vector<std::uint8_t>& message)
{
    if (!file.getRecordFromMessage(message))
    {
        task_info.error_code = make_error_code(ClientErrors::internal);
    }
}

void ModbusClient::createServerRequest(const TaskAttributes& attr)
{
    task_info.attributes = attr;
    task = std::async(&ModbusClient::callServerExchange, this);
}

void ModbusClient::callServerExchange()
{
    response_data.clear();
    try
    {
        serial_port.writeBinary(request_data);
    }
    catch (const std::system_error& e)
    {
        task_info.error_code = e.code();
        return;
    }
    try
    {
        serial_port.readBinary(response_data, task_info.attributes.length);
    }
    catch (const std::system_error& e)
    {
        task_info.error_code = e.code();
        return;
    }
}

} // namespace sm
