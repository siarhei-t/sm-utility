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

int main()
{
    const std::string port = "COM1";
    const std::uint8_t address = 1;
    sm::ServerData server_data;
    sm::Client client;
    
    std::error_code error;
    sp::PortConfig config;
    config.baudrate = sp::PortBaudRate::BD_19200;
    
    error = client.start(port);
    if(error){std::printf("failed to start client. \n"); return 0;}
    error = client.configure(config);
    if(error){std::printf("failed to configure client. \n"); return 0;}
    client.connect(address);
    if(error){std::printf("failed to conect to server. \n"); return 0;}
    
    std::printf("connected to server wit id : %d \n",address);
    client.getServerData(server_data);
    std::printf("device name : %s \n",server_data.data.boot_name);
    std::printf("boot version : %s \n",server_data.data.boot_version);
    std::printf("available ROM : %d KB \n",server_data.data.available_rom/1024);
    
    return 0;
}
