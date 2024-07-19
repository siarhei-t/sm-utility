/**
 * @file sm_utility.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <cstdint>
#include <iostream>
#include "lib/inc/sm_server.hpp"

int main(int argc, char* argv[])
{
    (void)(argc);
    (void)(argv);
    sm::ModbusServer server(1);
    std::uint8_t data[5] = {1,0,0,0,0};
    
    server.serverTask(data);

    std::cout<<"test \n";
}
