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
            serial_port.port.closePort();
            task_info.error_code = serial_port.open(device);
        }
    }

    return task_info.error_code;
}

std::error_code Client::configure(sp::PortConfig config)
{
    task_info.error_code = serial_port.setup(config);
    return task_info.error_code;
}

std::error_code Client::connect(const std::uint8_t address)
{
    // server is not selected yet
    if (server_id == not_connected)
    {
        addServer(address);
    }
    // current server has different address
    if (servers[server_id].info.addr != address)
    {
        addServer(address);
    }

    if (servers[server_id].info.status != ServerStatus::Available)
    {
        // ping server if it is not connected
        task_info = TaskInfo(ClientTasks::ping, 1);
        q_task.push([this]() { q_exchange.push([this] { ping(); }); });
        while (!task_info.done)
            ;
    }
    if (task_info.error_code)
    {
        return task_info.error_code;
    }
    if (servers[server_id].info.status == ServerStatus::Available)
    {
        // download registers
        task_info = TaskInfo(ClientTasks::regs_download, 1);
        q_task.push(
            [this]()
            { q_exchange.push([this] { readRegisters(0, amount_of_regs); }); });
        while (!task_info.done)
            ;
    }
    if (task_info.error_code)
    {
        return task_info.error_code;
    }
    // download file with general information
    // prepare to read
    task_info = TaskInfo(ClientTasks::reg_write, 1);
    q_task.push(
        [this]()
        {
            q_exchange.push(
                [this]
                {
                    writeRegister(static_cast<std::uint16_t>(
                                      ServerRegisters::file_control),
                                  file_read_prepare);
                });
        });
    while (!task_info.done)
        ;
    // reading
    task_info = TaskInfo();
    q_task.push([this]() { readFile(ServerFiles::info); });
    while (!task_info.done)
        ;
    return task_info.error_code;
}

std::error_code Client::eraseApp()
{
    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    if (server_id != not_connected)
    {
        if (servers[server_id].info.status == ServerStatus::Available)
        {
            // erase request
            task_info = TaskInfo(ClientTasks::reg_write, 1);
            q_task.push(
                [this]()
                {
                    q_exchange.push(
                        [this]
                        {
                            writeRegister(static_cast<std::uint16_t>(
                                              ServerRegisters::app_erase),
                                          app_erase_request);
                        });
                });
            while (!task_info.done)
                ;
            task_info = TaskInfo(ClientTasks::regs_download, 1);
            q_task.push(
                [this]() {
                    q_exchange.push([this]
                                    { readRegisters(0, amount_of_regs); });
                });
            while (!task_info.done)
                ;
            if (servers[server_id]
                    .regs[static_cast<int>(ServerRegisters::boot_status)] !=
                static_cast<std::uint16_t>(BootloaderStatus::empty))
            {
                task_info.error_code = make_error_code(ClientErrors::internal);
            }
        }
    }
    return task_info.error_code;
}

std::error_code Client::uploadApp(const std::string path_to_file)
{
    task_info.error_code = make_error_code(ClientErrors::server_not_connected);
    if (server_id != not_connected)
    {
        if (servers[server_id].info.status == ServerStatus::Available)
        {
            BootloaderStatus bootloader_status = static_cast<BootloaderStatus>(
                servers[server_id]
                    .regs[static_cast<int>(ServerRegisters::boot_status)]);
            // erase app file if we have some another state except
            // BootloaderStatus::empty
            if (bootloader_status != BootloaderStatus::empty)
            {
                (void)eraseApp();
                if (task_info.error_code)
                {
                    return task_info.error_code;
                }
            }
            std::uint8_t block_size = servers[server_id].regs[static_cast<int>(ServerRegisters::record_size)];
            if (file.fileWriteSetup(path_to_file, block_size))
            {
                // send file size
                task_info = TaskInfo(ClientTasks::reg_write, 1);
                q_task.push(
                    [this]()
                    {
                        q_exchange.push(
                            [this]
                            {
                                writeRegister(static_cast<std::uint16_t>(
                                                  ServerRegisters::app_size),
                                              file.getNumOfRecords());
                            });
                    });
                while (!task_info.done);
                // prepare to write
                task_info = TaskInfo(ClientTasks::reg_write, 1);
                q_task.push(
                    [this]()
                    {
                        q_exchange.push(
                            [this]
                            {
                                writeRegister(static_cast<std::uint16_t>(
                                                  ServerRegisters::file_control),
                                              file_write_prepare);
                            });
                    });
                while (!task_info.done);
                
                // writing
                task_info = TaskInfo();
                q_task.push([this,path_to_file]() { writeFile(ServerFiles::app); });
                while (!task_info.done);
                //read status back
                task_info = TaskInfo(ClientTasks::regs_download, 1);
                q_task.push(
                    [this]() {
                        q_exchange.push([this]
                                        { readRegisters(0, amount_of_regs); });
                    });
                while (!task_info.done);
                //check status again
                BootloaderStatus bootloader_status = static_cast<BootloaderStatus>(
                servers[server_id]
                    .regs[static_cast<int>(ServerRegisters::boot_status)]);
                // BootloaderStatus::empty
                if (bootloader_status != BootloaderStatus::ready)
                {
                    std::printf("Actual status : %d",(int)(bootloader_status));
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

void Client::disconnect()
{
    if (server_id != not_connected)
    {
        servers[server_id].info.status = ServerStatus::Unavailable;
        server_id = not_connected;
    }
}

void Client::addServer(const std::uint8_t address)
{
    auto it =
        std::find_if(servers.begin(), servers.end(),
                     [](ServerData& server) {
                         return server.info.status == ServerStatus::Unavailable;
                     });
    if (it != servers.end())
    {
        // use existed slot
        int index = std::distance(servers.begin(), it);
        servers[index] = ServerData();
        servers[index].info.addr = address;
        server_id = index;
    }
    else
    {
        // create new one
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
        while (!q_task.empty())
        {
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
                    std::cerr << e.what() << std::endl;
                }
                exchangeCallback();
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

void Client::writeRecord(const std::uint16_t file_id,
                         const std::uint16_t record_id,
                         const std::vector<std::uint8_t>& data)
{
    if (server_id != not_connected)
    {
        request_data = modbus_client.msgWriteFileRecord(
            servers[server_id].info.addr, file_id, record_id, data);
        // in case of success we expect message with the same length
        TaskAttributes attr = TaskAttributes(modbus::FunctionCodes::write_file,
                                             request_data.size());
        createServerRequest(attr);
    }
}

void Client::readRecord(const std::uint16_t file_id,
                        const std::uint16_t record_id,
                        const std::uint16_t length)
{
    if (server_id != not_connected)
    {
        request_data = modbus_client.msgReadFileRecord(
            servers[server_id].info.addr, file_id, record_id, length);
        // amount of half words + 1 byte for ref type + 1 byte for data length
        // + 1 byte for resp length + 1 byte for func + modbus required part
        TaskAttributes attr = TaskAttributes(
            modbus::FunctionCodes::read_file,
            static_cast<size_t>(modbus_client.getRequriedLength() +
                                (length * 2) + 4));
        createServerRequest(attr);
    }
}

void Client::writeRegister(const std::uint16_t address,
                           const std::uint16_t value)
{
    if (server_id != not_connected)
    {
        request_data = modbus_client.msgWriteRegister(
            servers[server_id].info.addr, address, value);
        // in case of success we expect message with the same length
        TaskAttributes attr = TaskAttributes(
            modbus::FunctionCodes::write_register, request_data.size());
        createServerRequest(attr);
    }
}

void Client::readRegisters(const std::uint16_t address,
                           const std::uint16_t quantity)
{
    if (server_id != not_connected)
    {
        request_data = modbus_client.msgReadRegisters(
            servers[server_id].info.addr, address, quantity);
        // amount of 16 bit registers + 1 byte for length + 1 byte for func +
        // modbus required part
        TaskAttributes attr = TaskAttributes(
            modbus::FunctionCodes::read_registers,
            static_cast<size_t>(modbus_client.getRequriedLength() +
                                (quantity * 2) + 2));
        createServerRequest(attr);
    }
}

void Client::writeFile(const ServerFiles file_id)
{
    if (server_id == not_connected)
    {
        return;
    }
    std::uint8_t block_size =
        servers[server_id].regs[static_cast<int>(ServerRegisters::record_size)];
    std::uint16_t id = static_cast<std::uint16_t>(file_id);
    auto num_of_records = file.getNumOfRecords();
    task_info = TaskInfo(ClientTasks::app_upload, num_of_records);
    for (auto i = 0; i < num_of_records; ++i)
    {
        std::vector<uint8_t> data;
        data.assign(&(file.getData()[i * block_size]),
                    &(file.getData()[i * block_size]) + block_size);
        
        q_exchange.push(
            [this, data, id, i]
            { writeRecord(id, static_cast<std::uint16_t>(i), data); });
    }
}

void Client::readFile(const ServerFiles file_id)
{
    if (server_id == not_connected)
    {
        return;
    }
    size_t file_size = 0;
    ClientTasks file_task = ClientTasks::undefined;

    switch (file_id)
    {
        case ServerFiles::app:
            file_size = servers[server_id]
                            .regs[static_cast<int>(ServerRegisters::app_size)];
            file_task = ClientTasks::app_download;
            break;

        case ServerFiles::info:
            file_size = sizeof(BootloaderInfo);
            file_task = ClientTasks::info_download;
            break;
    }
    if (file.fileReadSetup(file_size, servers[server_id].regs[static_cast<int>(
                                          ServerRegisters::record_size)]))
    {
        auto num_of_records = file.getNumOfRecords();
        task_info = TaskInfo(file_task, num_of_records);
        for (auto i = 0; i < num_of_records; ++i)
        {
            auto words_in_record = file.getActualRecordLength(i) / 2;
            q_exchange.push(
                [this, words_in_record, file_id, i]
                {
                    readRecord(static_cast<std::uint16_t>(file_id),
                               static_cast<std::uint16_t>(i), words_in_record);
                });
        }
    }
}

void Client::ping()
{
    if (server_id != not_connected)
    {
        std::uint8_t address = servers[server_id].info.addr;
        std::uint8_t function =
            static_cast<uint8_t>(modbus::FunctionCodes::undefined);
        std::vector<uint8_t> message{0x00, 0x00, 0x00, 0x00};
        request_data = modbus_client.msgCustom(address, function, message);
        // 1 byte for exception + 1 byte for func + modbus required part
        TaskAttributes attr = TaskAttributes(
            modbus::FunctionCodes::undefined,
            static_cast<size_t>(modbus_client.getRequriedLength() + 2));
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
        if (message[id_start] >
            (message.size() - (modbus::crc_size + modbus::address_size +
                               modbus::function_size + 1)))
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
    if (modbus_client.isChecksumValid(responce_data))
    {
        std::vector<std::uint8_t> message;
        modbus_client.extractData(responce_data, message);
        if (responce_data.size() != task_info.attributes.length)
        {
            task_info.error_code =
                make_error_code(ClientErrors::server_exception);
        }
        else
        {
            switch (task_info.task)
            {
                case ClientTasks::ping:
                    servers[server_id].info.status = ServerStatus::Available;
                    break;

                case ClientTasks::regs_download:
                    readRegs(servers[server_id], message);
                    break;

                case ClientTasks::info_download:
                    if (!file.getRecordFromMessage(message))
                    {
                        task_info.error_code =
                            make_error_code(ClientErrors::internal);
                    }
                    else
                    {
                        if (file.isFileReady())
                        {
                            std::memcpy(&servers[server_id].data,
                                        &file.getData()[0],
                                        sizeof(BootloaderInfo));
                        }
                    }
                    break;
                case ClientTasks::app_upload:
                    std::printf("record written : %d \n",task_info.counter);
                    break;

                default:
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
    // read from server
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
