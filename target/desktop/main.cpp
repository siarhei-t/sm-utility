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
#include "sm_modbus.hpp"


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
    // (3) create master and slave server instances
    client.addServer(address);
    // ping server, expected answer with modbus::Exceptions::exception_1
    error_code = client.taskPing(address);
    if (error_code)
    {
        std::cout<<"error: "<<error_code.message()<<"\n";
    }
    error_code = client.taskReadRegisters(address,modbus::holding_regs_offset + 4,1);
    if (error_code)
    {
        std::cout<<"error: "<<error_code.message()<<"\n";
    }

}
