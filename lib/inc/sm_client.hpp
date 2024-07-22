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
#include <future>
#include <memory>
#include <queue>
#include <thread>

#include "../../external/simple-serial-port-1.03/lib/inc/serial_port.hpp"
#include "../inc/sm_error.hpp"
#include "../inc/sm_file.hpp"
#include "../inc/sm_modbus.hpp"

namespace sm
{
//////////////////////////////SERVER CONSTANTS//////////////////////////////////
constexpr int boot_version_size = 17;
constexpr int boot_name_size = 33;
constexpr int not_connected = 255;
constexpr std::uint16_t file_read_prepare = 1;
constexpr std::uint16_t file_write_prepare = 2;
constexpr std::uint16_t app_erase_request = 1;
constexpr std::uint16_t app_start_request = 1;
////////////////////////////////////////////////////////////////////////////////

enum class ServerRegisters
{
    file_control = 0,
    prepare_to_update,
    app_erase,
    record_size,
    record_counter,
    boot_status,
    confirm_update,
    gateway_buffer_size,
    gateway_file_control,
    count
};

enum class BootloaderStatus
{
    unknown = 0,
    empty = 1,
    ready = 2,
    error = 3
};

enum class ServerFiles
{
    application = 1,
    server_metadata = 2
};

enum class ClientTasks
{
    undefined,
    regs_read,
    reg_write,
    file_read,
    file_write,
    ping,//extra command, FunctionCodes::undefined used
    app_start//extra command, the same as reg_write, but no responce expected
};

struct TaskAttributes
{
    TaskAttributes() = default;
    TaskAttributes(modbus::FunctionCodes code, size_t length)
        : code(code), length(length)
    {
    }
    modbus::FunctionCodes code = modbus::FunctionCodes::undefined;
    size_t length = 0;
};

struct TaskInfo
{
    TaskInfo() = default;
    TaskInfo(ClientTasks task, int num_of_exchanges, int index)
        : task(task), num_of_exchanges(num_of_exchanges), index(index){};
    ClientTasks task = ClientTasks::undefined;
    TaskAttributes attributes;
    std::error_code error_code;
    int num_of_exchanges = 0;
    int counter = 0;
    int index = -1;
    std::atomic<bool> done = false;
    void reset(ClientTasks task = ClientTasks::undefined, int num_of_exchanges = 0, int index = -1)
    {
        this->task = task;
        this->num_of_exchanges = num_of_exchanges;
        this->index = index;
        counter = 0;
        done = false;
        attributes = TaskAttributes();
        error_code = std::error_code();
    }
};

#pragma pack(push)
#pragma pack(2)
struct BootloaderInfo
{
    char boot_version[boot_version_size];
    char boot_name[boot_name_size];
    std::uint32_t available_rom;
};
#pragma pack(pop)

enum class ServerStatus
{
    Unavailable,
    Available
};

struct ServerInfo
{
    std::uint8_t addr = 0;
    std::uint8_t gateway_addr = 0;
    ServerStatus status = ServerStatus::Unavailable;
};

struct ServerData
{
    ServerInfo info;
    std::uint16_t regs[static_cast<std::size_t>(ServerRegisters::count)] = {};
    BootloaderInfo data = {};
};

class Client
{
public:
    Client() : client_thread(&Client::clientThread, this) {}
    ~Client();
    /// @brief stop client, close port
    void stop();
    /// @param addr server address
    /// @param gateway_addr address of gateway server
    void addServer(const std::uint8_t addr, const std::uint8_t gateway_addr = 0);
    /// @brief start client
    /// @param device device name to use
    /// @return error code
    std::error_code start(std::string device);
    /// @brief client device configure
    /// @param config used config
    /// @return error code
    std::error_code configure(sp::PortConfig config);
    /// @brief add server to the internal servers list
    /// @brief connect to server with selected id
    /// @param address server address
    /// @return error code
    std::error_code connect(const std::uint8_t address);
    /// @brief erase firmware on the server
    /// @return error code
    std::error_code eraseApp(const std::uint8_t address);
    /// @brief upload new firmware
    /// @param path_to_file path to file
    /// @return error code
    std::error_code uploadApp(const std::uint8_t address, const std::string path_to_file);
    /// @brief load last received server data
    /// @param @param server address in Modbus allpication area
    /// @param data reference to struct to save
    void getServerData(const std::uint8_t address, ServerData& data);
    /// @brief get actual running task progress in %
    /// @return value from 0 to 100
    int getActualTaskProgress() const;

private:
    /// @brief buffer for request message data
    std::vector<std::uint8_t> request_data;
    /// @brief buffer for response message data
    std::vector<std::uint8_t> responce_data;
    /// @brief modbus protocol message generator
    modbus::ModbusClient modbus_client;
    /// @brief file control instance
    File file;
    /// @brief serial port instance
    sp::SerialPort serial_port;
    /// @brief vector with actual available modbus devices
    std::vector<ServerData> servers;
    /// @brief client-server data thread
    std::thread client_thread;
    /// @brief logic semaphore to stop client_thread
    std::atomic<bool> thread_stop{false};
    /// @brief async client task variable
    std::future<void> task;
    /// @brief info about actual pending task and function
    TaskInfo task_info = TaskInfo(ClientTasks::undefined, 0,-1);
    /// @brief queue with client-server exchanges
    std::queue<std::function<void()>> q_exchange;
    /// @brief queue with client tasks
    std::queue<std::function<void()>> q_task;
    /// @brief ping server selected by address
    /// @param dev_addr server address
    /// @return error code
    std::error_code taskPing(const std::uint8_t dev_addr);
    /// @brief write new value to the register on the server selected by address
    /// @param dev_addr server address
    /// @param reg_addr register address
    /// @param value new value
    /// @return error code
    std::error_code taskWriteRegister(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t value);
    /// @brief read registers from the server selected by address
    /// @param dev_addr server address
    /// @param reg_addr register start address
    /// @param quantity amount of registers to read
    /// @return error code
    std::error_code taskReadRegisters(const std::uint8_t dev_addr, const std::uint16_t reg_addr, const std::uint16_t quantity);
    /// @brief read file from the server selected by address
    /// @param dev_addr server address
    /// @param file_id file id
    /// @return error code
    std::error_code taskReadFile(const std::uint8_t dev_addr, const ServerFiles file_id);
    /// @brief write file stored in file control instance to the server selected by address
    /// @param dev_addr server address
    /// @return error code
    std::error_code taskWriteFile(const std::uint8_t dev_addr);
    /// @brief get expected file size based on server predefined logic
    /// @param file_id file id in Modbus application layer
    /// @return file size in bytes
    static size_t getFileSize(const ServerFiles file_id);
    /// @brief get expected file size based on server predefined logic
    /// @param task_progress actual task progress in %
    static void printProgressBar(const int task_progress);
    /// @brief get server index in servers vector
    /// @param address server address
    /// @return actual index or -1 if server not exist
    int getServerIndex(const std::uint8_t address);
    /// @brief handler for client_thread
    void clientThread();
    /// @brief async call for callServerExchange method
    /// @param attr new task attributes
    void createServerRequest(const TaskAttributes& attr);
    /// @brief call request/response exchange on data prepared in request_data
    void callServerExchange();
    /// @brief callback called for every function in q_exchange
    void exchangeCallback();
    /// @brief callback called for every ClientTasks::file_read
    /// @param message reference to a vector with the message read
    /// @param index server index in servers vector
    void fileReadCallback(std::vector<std::uint8_t>& message,const int index);
};
} // namespace sm

#endif // SM_CLIENT_H
