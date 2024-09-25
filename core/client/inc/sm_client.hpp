/**
 * @file sm_client.hpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
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

#include "../../common/sm_modbus.hpp"
#include "../../external/simple-serial-port/inc/serial_port.hpp"
#include "../inc/sm_file.hpp"
#include "../inc/sm_message.hpp"

namespace sm
{

constexpr int server_not_found = -1;
constexpr int default_task_wait_delay_ms = 50;
constexpr int task_complete_value = 100;
constexpr int task_not_started_value = 0;
constexpr std::uint16_t file_read_prepare = 1;
constexpr std::uint16_t file_write_prepare = 2;
constexpr std::uint16_t app_erase_request = 1;

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
    TaskInfo(ClientTasks task, int num_of_exchanges, int index) : task(task), num_of_exchanges(num_of_exchanges), index(index) {};
    ClientTasks task = ClientTasks::undefined;
    TaskAttributes attributes;
    std::error_code error_code;
    int num_of_exchanges = 0;
    int counter = 0;
    int index = -1;
    bool is_printable = false;
    std::atomic<bool> done{false};
    void reset(ClientTasks task = ClientTasks::undefined, int num_of_exchanges = 0, int index = -1, bool is_printable = false)
    {
        this->task = task;
        this->num_of_exchanges = num_of_exchanges;
        this->index = index;
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
    std::uint8_t gateway_addr = 0;
    // record size will be configured automatically if register with ServerRegisters::record_size index will be read
    std::uint8_t record_size = 0;
    // the server will be marked as available if ClientTasks::ping completes successfully
    ServerStatus status = ServerStatus::unavailable;
};

struct ServerRegisters
{
    std::uint16_t reg_start_address = modbus::holding_regs_offset;
    std::vector<std::uint16_t> values;
};

struct ServerData
{
    ServerInfo info;
    ServerRegisters registers;
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
    /**
     * @brief stop client, close serial port
     *
     */
    void stop();
    /**
     * @brief adds erver to the vector with used servers
     *
     * @param dev_addr server address in Modbus application layer
     * @param gateway_addr gateway address in case of gatewayed access
     */
    void addServer(const std::uint8_t dev_addr, const std::uint8_t gateway_addr = 0);
    /**
     * @brief start client on selected serial port
     *
     * @param device port name
     * @return std::error_code
     */
    std::error_code start(std::string device);
    /**
     * @brief setup serial port
     *
     * @param config port configuration
     * @return std::error_code
     */
    std::error_code configure(sp::PortConfig config);
    /**
     * @brief ping server, mark it available if exist
     *
     * @param dev_addr server address in Modbus application layer
     * @return std::error_code
     */
    std::error_code taskPing(const std::uint8_t dev_addr);
    /**
     * @brief write new value to server register
     *
     * @param dev_addr server address in Modbus application layer
     * @param reg_addr register address in Modbus application layer
     * @param value new register address
     * @return std::error_code
     */
    std::error_code taskWriteRegister(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t value, const bool print_progress = false);
    /**
     * @brief read registers from the server
     *
     * @param dev_addr server address in Modbus application layer
     * @param reg_addr register start address in Modbus application layer
     * @param quantity amount of registers
     * @return std::error_code
     */
    std::error_code taskReadRegisters(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t quantity, const bool print_progress = false);
    /**
     * @brief read file from the server
     *
     * @param dev_addr server address in Modbus application layer
     * @param file_id file id in Modbus application layer
     * @param file_size expected file size
     * @return std::error_code
     */
    std::error_code taskReadFile(const std::uint8_t dev_addr, const std::uint16_t file_id, const std::size_t file_size, const bool print_progress = false);
    /**
     * @brief write file to the server
     *
     * @param dev_addr server address in Modbus application layer
     * @return std::error_code
     */
    std::error_code taskWriteFile(const std::uint8_t dev_addr, const bool print_progress = false);
    /**
     * @brief Get the actual task progress
     *
     * @return int value from task_not_started_value to task_complete_value
     */
    int getActualTaskProgress() const;
    /**
     * @brief Get vector with actual values of server registers
     *
     * @param dev_addr server address in Modbus application layer
     * @param registers reference to vector with registers
     */
    void getLastServerRegList(const std::uint8_t dev_addr, ServerRegisters& registers);
    /**
     * @brief forced setup server as available to skip ClientTasks::ping task 
     * 
     * @param dev_addr server address in Modbus application layer
     * @return true in case of success
     * @return false if server was not found
     */
    bool setServerAsAvailable(const std::uint8_t dev_addr);
    /**
     * @brief forced setup max record size for the server
     * 
     * @param dev_addr server address in Modbus application layer
     * @param record_size record size in bytes
     * @return true in case of success
     * @return false if server was not found
     */
    bool setServerRecordMaxSize(const std::uint8_t dev_addr, const std::uint8_t record_size);

private:
    std::vector<std::uint8_t> request_data;
    std::vector<std::uint8_t> responce_data;
    modbus::ModbusMessage modbus_message = modbus::ModbusMessage(modbus::ModbusMode::rtu);
    std::vector<ServerData> servers;
    std::thread client_thread;
    std::atomic<bool> thread_stop{false};
    std::future<void> task;
    TaskInfo task_info = TaskInfo(ClientTasks::undefined, 0, -1);
    std::queue<std::function<void()>> q_exchange;
    std::queue<std::function<void()>> q_task;
    /**
     * @brief get server index in internal vector with servers
     *
     * @param dev_addr server address in Modbus application layer
     * @return int index in servers array, server_not_found if server not exist
     */
    int getServerIndex(const std::uint8_t dev_addr) const;
    /**
     * @brief get expected server response length
     *
     * @param task task to execute
     * @param extra extra length in bytes to receive
     * @return size in bytes
     */
    size_t getExpectedLength(const ClientTasks task, const size_t extra = 0) const;
    /**
     * @brief handler for client_thread
     *
     */
    void clientThread();
    /**
     * @brief Async call for callServerExchange method
     *
     * @param attr reference to the new task attributes
     */
    void createServerRequest(const TaskAttributes& attr);
    /**
     * @brief call request/response exchange on data prepared in request_data
     *
     */
    void callServerExchange();
    /**
     * @brief callback called for every function in q_exchange
     *
     */
    void exchangeCallback();
    /**
     * @brief callback for server file read/write processing
     *
     * @param message received message with record
     */
    void fileReadCallback(std::vector<std::uint8_t>& message);
    /**
     * @brief print task progress to stdout
     * 
     */
    void printProgressBar(const int task_progress);
};

} // namespace sm

#endif // SM_CLIENT_H
