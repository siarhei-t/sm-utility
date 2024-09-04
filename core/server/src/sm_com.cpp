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

namespace sm
{

void Com::startReadData(std::uint8_t data[], const size_t amount)
{
    platformReadData(data,amount);
}

void Com::startSendData(std::uint8_t data[], const size_t amount)
{
    platformSendData(data,amount);
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define, typename c>
void DataNode<amount_of_regs,amount_of_files,record_define,c>::start()
{
    com.startReadData(buffer,server.getReceiveBufferSize());
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define, typename c>
void DataNode<amount_of_regs,amount_of_files,record_define,c>::loop()
{
    
}

} // namespace sm
