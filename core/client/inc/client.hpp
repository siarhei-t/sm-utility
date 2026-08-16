/**
 * @file client.hpp
 *
 * @brief  header for client.cpp
 *
 */

#ifndef SM_CLIENT_H
#define SM_CLIENT_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <queue>
#include <thread>
#include <vector>

#include "../../common/modbus.hpp"
#include "../../external/simple-serial-port/inc/serial_port.hpp"
#include "../inc/file.hpp"
#include "../inc/message.hpp"

namespace sm
{

constexpr int server_not_found = -1;
constexpr int default_task_wait_delay_ms = 50;
constexpr int task_complete_value = 100;
constexpr int task_not_started_value = 0;

enum class ClientTasks
{
    undefined,
    regs_read,
    reg_write,
    file_read,
    file_write,
    ping, // extra command, FunctionCodes::undefined used
};

struct TaskAttributes
{
    TaskAttributes() = default;
    TaskAttributes(modbus::FunctionCodes code, size_t length) : code(code), length(length) {}
    modbus::FunctionCodes code = modbus::FunctionCodes::undefined;
    size_t length = 0;
};

struct TaskInfo
{
    TaskInfo() = default;
    TaskInfo(ClientTasks task, int num_of_exchanges) : task(task), num_of_exchanges(num_of_exchanges) {};
    ClientTasks task = ClientTasks::undefined;
    TaskAttributes attributes;
    std::error_code error_code;
    int num_of_exchanges = 0;
    int counter = 0;
    bool is_printable = false;
    std::atomic<bool> done{false};
    void reset(ClientTasks task = ClientTasks::undefined, int num_of_exchanges = 0, bool is_printable = false)
    {
        this->task = task;
        this->num_of_exchanges = num_of_exchanges;
        this->is_printable = is_printable;
        counter = 0;
        done.store(false, std::memory_order_relaxed);
        attributes = TaskAttributes();
        error_code = std::error_code();
    }
};

enum class ServerStatus
{
    unavailable,
    available
};

struct ServerInfo
{
    std::uint8_t addr = 0;
    // record size will be configured automatically if register with ServerRegisters::record_size index will be read
    std::uint16_t record_size = 0;
    // start address of last read registers vector
    std::uint16_t reg_start_address = 0;
    // vector contains all registers read
    std::vector<uint8_t> regs;
    // the server will be marked as available if ClientTasks::ping completes successfully
    ServerStatus status = ServerStatus::unavailable;
};

class ModbusClient
{

public:
    ModbusClient() : client_thread(&ModbusClient::clientThread, this) {}
    ~ModbusClient()
    {
        thread_stop.store(true, std::memory_order_relaxed);
        client_thread.join();
    }
    sp::SerialPort serial_port;
    File file;

    void stop();
    void setAddress(const std::uint8_t addr) { server.addr = addr; }

    std::error_code start(std::string device);
    std::error_code configure(sp::PortConfig config);

    std::error_code taskPing(const std::uint8_t dev_addr);
    std::error_code taskWriteRegister(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t value, const bool print_progress = false);
    std::error_code taskReadRegisters(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t quantity,
                                      const bool print_progress = false);
    std::error_code taskReadFile(const std::uint8_t dev_addr, const std::uint16_t file_id, const std::size_t file_size, const bool print_progress = false);
    std::error_code taskWriteFile(const std::uint8_t dev_addr, const bool print_progress = false);

    int getActualTaskProgress() const;

private:
    std::vector<std::uint8_t> request_data;
    std::vector<std::uint8_t> response_data;
    modbus::ModbusMessage modbus_message = modbus::ModbusMessage(modbus::ModbusMode::rtu);
    ServerInfo server;
    std::thread client_thread;
    std::atomic<bool> thread_stop{false};
    std::future<void> task;
    TaskInfo task_info{ClientTasks::undefined, 0};
    std::queue<std::function<void()>> q_exchange;
    std::queue<std::function<void()>> q_task;

    size_t getExpectedLength(const ClientTasks task, const size_t extra = 0) const;
    void clientThread();
    void createServerRequest(const TaskAttributes& attr);
    void callServerExchange();
    void exchangeCallback();
    void fileReadCallback(std::vector<std::uint8_t>& message);
    void printProgressBar(const int task_progress);
};

} // namespace sm

#endif // SM_CLIENT_H
