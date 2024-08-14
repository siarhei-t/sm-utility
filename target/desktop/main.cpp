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
    // (3) create master and slave server instances
    client.addServer(1);

    error_code = make_error_code(sm::ClientErrors::server_not_connected);
    // ping server, expected answer with modbus::Exceptions::exception_1
    error_code = client.taskPing(1);
    if (error_code)
    {
        std::cout<<"error: "<<error_code.message()<<"\n";
    }
}
