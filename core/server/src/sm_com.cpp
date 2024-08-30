/**
 * @file
 *
 * @brief
 *
 * @author
 *
 */

#include <array>
#include "../inc/sm_com.hpp"
#include "../../common/sm_modbus.hpp"

namespace
{
std::array<std::uint8_t, modbus::max_adu_size> buffer;
}

namespace sm
{

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define, typename c>
std::uint8_t* DataNode<amount_of_regs,amount_of_files,record_define,c>::getBufferPtr() const
{
    return buffer.data();
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define, typename c>
void DataNode<amount_of_regs,amount_of_files,record_define,c>::loop()
{
    
}

} // namespace sm
