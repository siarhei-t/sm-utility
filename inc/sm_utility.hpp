/**
 * @file sm_utility.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#ifndef SM_UTILITY_H
#define SM_UTILITY_H

#include "../inc/sm_modbus.hpp"

namespace sm
{
    //////////////////////////////SERVER CONSTANTS//////////////////////////////////
    constexpr int boot_version_size = 17;
    constexpr int boot_name_size    = 33;
    constexpr int amount_of_regs    = 7;
    ////////////////////////////////////////////////////////////////////////////////
    
    #pragma pack(push)
    #pragma pack(2)
    struct BootloaderInfo
    {
        char boot_version[boot_version_size];
        char boot_name[boot_name_size];
        std::uint32_t available_rom; 
    };
    #pragma  pack(pop)

    enum class ServerStatus
    {
        Unavailable,
        Available
    };
    
    struct ServerInfo
    {
        std::uint8_t addr;
        ServerStatus status;
        std::uint16_t regs[amount_of_regs];
        BootloaderInfo data;
    };
    

}

#endif //SM_UTILITY_H
