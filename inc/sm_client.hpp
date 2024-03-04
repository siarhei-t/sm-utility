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

#include "../inc/sm_modbus.hpp"

namespace sm
{
    //////////////////////////////SERVER CONSTANTS//////////////////////////////////
    constexpr int boot_version_size = 17;
    constexpr int boot_name_size    = 33;
    constexpr int amount_of_regs    = 7;
    constexpr int not_connected     = -1;
    ////////////////////////////////MODBUS CONSTANTS////////////////////////////////
    constexpr int crc_size         = 2;
    constexpr int address_size     = 1;
    constexpr int function_size    = 1;
    constexpr int rtu_start_size   = 4;
    constexpr int rtu_stop_size    = 4;
    constexpr int ascii_start_size = 1;
    constexpr int ascii_stop_size  = 2;
    constexpr int rtu_msg_edge     = (rtu_start_size + rtu_stop_size);
    constexpr int ascii_msg_edge   = (ascii_start_size + ascii_stop_size);
    constexpr int rtu_adu_size     = (rtu_msg_edge + crc_size + address_size);
    constexpr int ascii_adu_size   = (ascii_msg_edge + crc_size + address_size);                               
    ////////////////////////////////////////////////////////////////////////////////
    
    enum class ClientTasks
    {
        undefined,
        ping,
        appErase, 
        app_upload,
        app_download,
        app_start
    };

    struct TaskAttributes
    {
        modbus::FunctionCodes code = modbus::FunctionCodes::undefined;
        size_t length = 0;
    };

    struct TaskInfo
    {
        ClientTasks task = ClientTasks::undefined;
        TaskAttributes attributes;
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
            /// @brief connect to server with selected id
            /// @param address server address
            void connect(const std::uint8_t address);
            void disconnect();

        private:
            /// @brief buffer for request message data
            std::vector<std::uint8_t> request_data;
            /// @brief buffer for response message data
            std::vector<std::uint8_t> responce_data;
            /// @brief modbus protocol message generator
            modbus::ModbusClient modbus_client;
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
            TaskInfo task_info;
            
            void ping();
            /// @brief create new server instance
            /// @param address server address
            void addServer(const std::uint8_t address);
            /// @brief handler for client_thread
            void clientThread();
            /// @brief async call for callServerExchange method
            /// @param attr new task attributes
            void createServerRequest(const TaskAttributes& attr);
            /// @brief call request/response exchange on data prepared in request_data
            /// @param resp_length expected responce length
            void callServerExchange(const size_t resp_length);
    };
}

#endif //SM_CLIENT_H
