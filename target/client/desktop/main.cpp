/**
 * @file sm_utility.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */


#include <iostream>
#include <system_error>
#include "../../../core/client/inc/sm_client.hpp"

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::printf("incorrect agruments list passed, exit...\n");
        return 0;
    }
    sm::ModbusClient client;
    sp::PortConfig config;
    std::string path_to_port = argv[1];
    std::string address_str  = argv[2];
    std::uint8_t address;
    
    try
    {
        auto number = std::stoi(address_str);
        if(number > modbus::max_rtu_address)
        {
            std::cout <<"out of range address passed, exit...\n";
            return 0;
        }
        else if(number < modbus::min_rtu_address)
        {
            std::cout <<"broadcast is not supported, exit...\n";
            return 0;
        }
        else
        {
            address = static_cast<std::uint8_t>(number);
        }
    }
    catch (std::invalid_argument const& ex)
    {
        std::cout <<"invalid argumant passed, exit...\n";
        return 0;
    }
    
    //FIXME: fix it to read config from command line int the future 
    config.baudrate = sp::PortBaudRate::BD_57600;
    config.timeout_ms = 2000;

    auto error_code = client.start(path_to_port);
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

    // create server instance
    client.addServer(address);
    
    // ping server, expected answer with modbus::Exceptions::exception_1
    error_code = client.taskPing(address);
    if (error_code)
    {
        std::cout<<"error: "<<error_code.message()<<"\n";
    }

    // read first 6 registers from the server
    error_code = client.taskReadRegisters(address,modbus::holding_regs_offset + 6,1);
    if (error_code)
    {
        std::cout<<"error: "<<error_code.message()<<"\n";
    }

}
