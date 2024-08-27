/**
 * @file sm_utility.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */


#include <iostream>

#include "../../../core/server/inc/sm_server.hpp"


int main(int argc, char* argv[])
{
    (void)(argc);
    (void)(argv);
    sm::ModbusServer<5, 1, 100> test(1);

}
