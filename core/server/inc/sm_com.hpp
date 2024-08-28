/**
 * @file sm_com.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_COM_HPP
#define SM_COM_HPP

#include <cstddef>
#include <cstdint>
#include "../inc/sm_server.hpp"

namespace sm
{

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define> class Com
{
public:
    
private:
    std::uint8_t rx_length;
    std::uint8_t tx_length;
    ModbusServer<amount_of_regs,amount_of_files,record_define> server;

};

} // namespace sm

#endif // SM_COM_HPP
