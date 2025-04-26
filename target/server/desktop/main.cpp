/**
 * @file main.cpp
 *
 * @brief 
 *
 * @author
 *
 */

#include <cassert>
#include <iostream>
#include "platform.hpp"


constexpr std::uint8_t record_size = 208;

PlatformSupport platform_support;

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::printf("incorrect arguments list passed, exit...\n");
        return 0;
    }
    std::string path_to_port = argv[1];
    std::string address_str  = argv[2];
    std::uint8_t address;
    try
    {
        auto number = std::stoi(address_str);
        if( (number > modbus::max_rtu_address) || (number < modbus::min_rtu_address) )
        {
            std::cout <<"out of range address passed, exit...\n";
            return 0;
        }
        else
        {
            address = static_cast<std::uint8_t>(number);
        }
    }
    catch (std::invalid_argument const& ex)
    {
        std::cout <<"invalid argument passed, exit...\n";
        return 0;
    }
    
    //FIXME: fix it to read config from command line int the future 
    sp::PortConfig config;
    config.baudrate = sp::PortBaudRate::BD_57600;
    config.timeout_ms = 2000;
    
    platform_support.setPath(path_to_port);
    platform_support.setConfig(config);

    sm::DataNode<DesktopCom,DesktopTimer> data_node(address,record_size);

    data_node.start();
    data_node.loop();
}
