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
    client.connect(0x11);
    return 0;
}
