/**
 * @file read_write.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <iostream>
#include "../inc/sm_client.hpp"

int main(void)
{
    sm::Client client;
    
    std::error_code error;
    sp::PortConfig config;
    config.baudrate = sp::PortBaudRate::BD_19200;
    
    error = client.start("COM1");
    error = client.configure(config);

    client.connect(0x01);
    client.connect(0x02);
    
    return 0;
}
