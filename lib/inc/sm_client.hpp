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

#include <thread>
#include <atomic>
#include <memory>
#include <future>
#include <queue>

#include "../inc/sm_modbus.hpp"
#include "../../external/simple-serial-port/inc/serial_port.hpp"

namespace sm
{
    //////////////////////////////SERVER CONSTANTS//////////////////////////////////
    constexpr int boot_version_size = 17;
    constexpr int boot_name_size    = 33;
    constexpr int amount_of_regs    = 7;
    constexpr int not_connected     = -1;
    ////////////////////////////////////////////////////////////////////////////////
    
    enum class ClientTasks
    {
        undefined,
        ping,
        info_download,
        app_erase, 
        app_upload,
        app_download,
        app_start
    };

    struct TaskAttributes
    {
        TaskAttributes() = default;
        TaskAttributes(modbus::FunctionCodes code,size_t length):code(code),length(length){}
        modbus::FunctionCodes code = modbus::FunctionCodes::undefined;
        size_t length = 0;
    };
    
    struct TaskInfo
    {
        TaskInfo(ClientTasks task,int num_of_exchanges):task(task),num_of_exchanges(num_of_exchanges){};
        ClientTasks task;
        TaskAttributes attributes;
        std::error_code error_code;
        int num_of_exchanges;
        int counter = 0;
        bool done   = false;
        bool error  = false;
    };

    #pragma pack(push)
    #pragma pack(2)
    struct BootloaderInfo
    {
        char boot_version[boot_version_size];
        char boot_name[boot_name_size];
        std::uint32_t available_rom; 
    };
    #pragma  pack(pop)

    enum class ServerStatus
    {
        Unavailable,
        Available
    };

    struct ServerInfo
    {
        std::uint8_t addr = 0;
        ServerStatus status = ServerStatus::Unavailable;
    };

    struct ServerData
    {
        ServerInfo info;
        std::uint16_t regs[amount_of_regs] = {};
        BootloaderInfo data = {};
    };

    class Client
    {
        public:
            Client();
            ~Client();
            /// @brief start client
            /// @param device device name to use
            /// @return error code
            std::error_code& start(std::string device);
            /// @brief client device configure
            /// @param config used config
            /// @return error code
            std::error_code& configure(sp::PortConfig config);
            /// @brief connect to server with selected id
            /// @param address server address
            /// @return error code
            std::error_code& connect(const std::uint8_t address);
            /// @brief diconnect from server
            void disconnect();

        private:
            /// @brief buffer for request message data
            std::vector<std::uint8_t> request_data;
            /// @brief buffer for response message data
            std::vector<std::uint8_t> responce_data;
            /// @brief modbus protocol message generator
            modbus::ModbusClient modbus_client;
            /// @brief serial port instance
            sp::SerialPort serial_port;
            /// @brief vector with actual available modbus devices
            std::vector<ServerData> servers;
            /// @brief connected server index in servers
            int server_id = not_connected;
            /// @brief client-server data thread
            std::unique_ptr<std::thread> client_thread;
            /// @brief logic semaphore to stop client_thread
            std::atomic<bool> thread_stop{false};
            /// @brief async client task variable
            std::future<void> task;
            /// @brief info about actual pending task and function
            TaskInfo task_info = TaskInfo(ClientTasks::undefined,0);
            /// @brief queue with client-server exchanges
            std::queue<std::function<void()>> q_exchange;
            /// @brief queue with client tasks
            std::queue<std::function<void()>> q_task;
            /// @brief ping command
            void ping();
            /// @brief write record in file with new data
            /// @param file_id  file index
            /// @param record_id record index in file
            /// @param data new record data
            void writeRecord(const std::uint16_t file_id, const std::uint16_t record_id, const std::vector<std::uint8_t>& data);
            /// @brief read record from file
            /// @param file_id  file index
            /// @param record_id record index in file
            /// @param length length to read
            void readRecord(const std::uint16_t file_id, const std::uint16_t record_id, const std::uint16_t length);
            /// @brief write register command
            /// @param address address of register
            /// @param value register value
            void writeRegister(const std::uint16_t address, const std::uint16_t value);
            /// @brief read registers command
            /// @param address start address to read
            /// @param quantity amount of registers to read
            void readRegisters(const std::uint16_t address, const std::uint16_t quantity);
            /// @brief create new server instance
            /// @param address server address
            void addServer(const std::uint8_t address);
            /// @brief handler for client_thread
            void clientThread();
            /// @brief async call for callServerExchange method
            /// @param attr new task attributes
            void createServerRequest(const TaskAttributes& attr);
            /// @brief call request/response exchange on data prepared in request_data
            void callServerExchange();
            /// @brief callback called for every function in q_exchange
            void exchangeCallback();
    };
}

#endif //SM_CLIENT_H
