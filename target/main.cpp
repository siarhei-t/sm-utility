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
#include "lib/inc/sm_modbus.hpp"
#include "lib/inc/sm_server.hpp"

int main(int argc, char* argv[])
{
    (void)(argc);
    (void)(argv);
    sm::ModbusServer server(1);
    std::array<std::uint8_t, modbus::max_adu_size> data = {2,0,0,0,0};
    
    auto responce = server.serverTask(data.data());


    std::cout<<"test :"<< int(responce)<<"\n";
}
