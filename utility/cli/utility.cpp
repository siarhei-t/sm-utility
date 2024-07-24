/**
 * @file sm_utility.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include "../../core/client/inc/sm_client.hpp"
#include "../../core/client/inc/sm_error.hpp"

#pragma pack(push)
#pragma pack(2)
struct BootloaderInfo
{
    char boot_version[17];
    char boot_name[33];
    char serial_number[16];
    uint8_t random_nonce[12];
    std::uint32_t available_rom;
};
#pragma pack(pop)

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


class DesktopClient : public sm::ModbusClient
{
public:
    /// @brief add server to the internal servers list
    /// @brief connect to server with selected id
    /// @param address server address
    /// @return error code
    std::error_code connect(const std::uint8_t address);
    /// @brief upload new firmware
    /// @param path_to_file path to file
    /// @return error code
    std::error_code uploadApp(const std::uint8_t address, const std::string path_to_file, const std::uint8_t record_size);

};

std::error_code DesktopClient::connect(const std::uint8_t address)
{
    std::error_code error_code = make_error_code(sm::ClientErrors::server_not_connected);

    // (1) ping server, expected answer with exception type 1
    error_code = taskPing(address);
    if (error_code)
    {
        return error_code;
    }

    // (2) load all registers
    error_code = taskReadRegisters(address, modbus::holding_regs_offset, static_cast<std::uint16_t>(sm::ServerRegisters::count));
    if (error_code)
    {
        return error_code;
    }

    // (3.1) prepare to read
    error_code = taskWriteRegister(address, static_cast<std::uint16_t>(sm::ServerRegisters::file_control), sm::file_read_prepare);
    if (error_code)
    {
        return error_code;
    }

    // (3.2) file reading
    error_code = taskReadFile(address, static_cast<std::uint16_t>(ServerFiles::server_metadata),sizeof(BootloaderInfo));

    return error_code;
}

std::error_code DesktopClient::uploadApp(const std::uint8_t address, const std::string path_to_file, const std::uint8_t record_size)
{

    std::error_code error_code = make_error_code(sm::ClientErrors::server_not_connected);
    int index = getServerIndex(address);
    if (index == -1)
    {
        return error_code;
    }
    if (file.fileExternalWriteSetup(static_cast<std::uint16_t>(ServerFiles::application), path_to_file, record_size))
    {

        // (1) erase application
        error_code = taskWriteRegister(address, static_cast<std::uint16_t>(sm::ServerRegisters::app_erase), sm::app_erase_request);
        if (error_code)
        {
            return error_code;
        }
        // (2) send new file size
        std::uint16_t num_of_records = file.getNumOfRecords();

        error_code = taskWriteRegister(address, static_cast<std::uint16_t>(sm::ServerRegisters::prepare_to_update), num_of_records);
        if (error_code)
        {
            return error_code;
        }
        // (3) prepare to write
        error_code = taskWriteRegister(address, static_cast<std::uint16_t>(sm::ServerRegisters::file_control), sm::file_write_prepare);
        if (error_code)
        {
            return error_code;
        }
        // (4) file sending
        error_code = taskWriteFile(address);
        if (error_code)
        {
            return error_code;
        }
        // (5) read status back
        error_code = taskReadRegisters(address, modbus::holding_regs_offset, static_cast<std::uint16_t>(sm::ServerRegisters::count));
    }
    else
    {
        error_code = make_error_code(sm::ClientErrors::internal);
    }
    return error_code;
}

const std::string version = "0.01";

constexpr int master_address = 1;
constexpr int slave_address  = 2;

static void print_metadata(BootloaderInfo& server_data);

int main(int argc, char* argv[])
{
    //instance of serial port scanner
    sp::SerialDevice serial_device;
    //instance of modbus client
    DesktopClient client; 
    //serial port configuration
    sp::PortConfig config;
    //bootloader info
    BootloaderInfo info;
    // bootloader registers
    std::vector<std::uint16_t> registers;
    //setup serial port initial configuration
    config.baudrate = sp::PortBaudRate::BD_9600;
    config.timeout_ms = 2000;
    if(argc != 4)
    {
        std::printf("incorrect agruments list passed, exit...\n");
        return 0;
    }

    std::printf("\n");
    std::printf("*-------------------INFO----------------------*\n");
    std::printf("program name : EXOPULSE CU firmware update utility.\n");
    std::printf("version      : %s \n",version.c_str());
    //std::printf("*---------------------------------------------*\n");
    std::printf("*--------------CONFIGURATION------------------*\n");
    std::printf("serial port baudrate       : 115200\n");
    std::printf("master chip Modbus address : %d \n",master_address);
    std::printf("slave  chip Modbus address : %d \n",slave_address);
    std::printf("*---------------------------------------------*\n");
    // (1) check for serial ports in the system
    std::printf("scanning system serial ports...\n");
    serial_device.updateAvailableDevices();
    std::vector<std::string> devices = serial_device.getListOfAvailableDevices();
    std::cout<<"Available serial ports: ";
    for(std::string& device : devices){std::cout<<device<<" ";}
    std::printf("\n");
    std::printf("*---------------------------------------------*\n");
    std::string serial_port = argv[1];
    std::printf("trying to open serial port with name %s...\n",serial_port.c_str());
    // (2) trying to open serial port with passed address
    auto error_code = client.start(serial_port);
    if(error_code)
    {
        std::cout<<"failed to start client. \n"; 
        std::cout<<"error: "<<error_code.message()<<"\n";
        return 0;
    }
    error_code = client.configure(config);
    if(error_code)
    {
        std::cout<<"failed to configure client. \n";
        std::cout<<"error: "<<error_code.message()<<"\n";
        return 0;
    }
    else
    {
        std::cout<<"success, client started...\n";
    }
    // (3) create master and slave server instances
    client.addServer(master_address);
    client.addServer(slave_address,master_address);
    std::printf("*---------------------------------------------*\n");
    std::printf("trying to connect to master chip server...\n");
    // (4) connecting to master
    error_code = client.connect(master_address);
    if(error_code)
    {
        std::printf("failed to conect to server. \n");
        std::cout<<"error: "<<error_code.message()<<"\n";
        return 0;
    }
    else
    {
        std::memcpy(&info, &client.file.getData()[0], sizeof(BootloaderInfo));
        client.getLastServerRegList(master_address,registers);
        print_metadata(info);
    }
    /*
    std::printf("*---------------------------------------------*\n");
    std::printf("trying to connect to slave chip server...\n");
    // (4) connecting to slave
    error_code = client.connect(slave_address);
    if(error_code)
    {
        std::printf("failed to conect to server. \n");
        std::cout<<"error: "<<error_code.message()<<"\n";
        return 0;
    }
    else
    {
        client.getServerData(slave_address,servers[1]);
        print_metadata(servers[1]);
    }
    
    // (5) send new firmware to master chip
    std::printf("*---------------------------------------------*\n");
    std::printf("trying to update firmware on the master chip...\n");
    std::string master_firmware = argv[2];
    error_code = client.uploadApp(master_address,master_firmware);
    if(error_code)
    {
        std::printf("failed to upload firmware. \n");
        std::cout<<"error: "<<error_code.message()<<"\n";
        return 0;
    }
    else
    {
        std::cout<<"new firmware uploaded.\n";
    }
    
    // (6) send new firmware to slave chip
    std::printf("*---------------------------------------------*\n");
    std::printf("trying to update firmware on the slave chip...\n");
    std::string slave_firmware = argv[3];
    error_code = client.uploadApp(slave_address,slave_firmware);
    if(error_code)
    {
        std::printf("failed to upload firmware. \n");
        std::cout<<"error: "<<error_code.message()<<"\n";
    }
    else
    {
        std::cout<<"new firmware uploaded.\n";
    }
    
    std::printf("*---------------------------------------------*\n");
    std::printf("success, program exit...\n");
    */
    return 0;
}

static void print_metadata(BootloaderInfo& info)
{
    std::printf("*---------------------------------------------*\n");
    std::printf("device name      : %s \n",info.boot_name);
    std::printf("boot version     : %s \n",info.boot_version);
    std::printf("available ROM    : %d KB \n",info.available_rom/1024);
    
    std::printf("serial number    : ");
    for(std::size_t i = 0; i < sizeof(info.serial_number); ++i)
    {
        std::printf("%d ",info.serial_number[i]);
    }
    std::printf("\n");
    std::printf("temporary nonce  : ");

    for(std::size_t i = 0; i < sizeof(info.random_nonce); ++i)
    {
        std::printf("%d ",info.random_nonce[i]);
    }
    std::printf("\n");
    std::printf("*---------------------------------------------*\n");
}
