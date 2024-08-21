/**
 * @file sm_utility.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */


#include <iostream>

#include "../../core/client/inc/sm_client.hpp"
#include "../../core/client/inc/sm_error.hpp"

struct BootloaderInfo
{
    char boot_version[17];
    char boot_name[33];
    char serial_number[16];
    std::uint8_t random_salt[12];
    std::uint8_t device_id[16];

};

std::vector<std::uint8_t> nonce = {1,2,3,4,5,6,7,8,9,10,11,12};

int main(int argc, char* argv[])
{
    (void)(argc);
    (void)(argv);
     //serial port configuration
    sp::PortConfig config;
    //hardcoded port baudrate
    config.baudrate = sp::PortBaudRate::BD_57600;
    config.timeout_ms = 2000;

    sm::ModbusClient client;
    auto error_code = client.start("COM4");
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
    std::uint8_t address = 1;
    std::uint8_t gateway_address = 0;
    // (3) create master and slave server instances
    client.addServer(gateway_address);
    client.addServer(address,gateway_address);

    error_code = make_error_code(sm::ClientErrors::server_not_connected);
    // ping server, expected answer with modbus::Exceptions::exception_1
    error_code = client.taskPing(address);
    if (error_code)
    {
        std::cout<<"error: "<<error_code.message()<<"\n";
    }
    error_code = client.taskReadRegisters(address, modbus::holding_regs_offset, 7);
    if (error_code)
    {
        std::cout<<"error: "<<error_code.message()<<"\n";
    }
    std::vector<std::uint16_t> regs;
    client.getLastServerRegList(address, regs);
    std::uint16_t record_size = regs[3];
    error_code = client.taskReadFile(address, 2,sizeof(BootloaderInfo));
    if (error_code)
    {
        std::cout<<"error: "<<error_code.message()<<"\n";
    }

    if (client.file.fileWriteSetupFromMemory(4, nonce, record_size))
    {
        // file sending, we send what we actually have in buffer
        error_code = client.taskWriteFile(address);
    }
}
