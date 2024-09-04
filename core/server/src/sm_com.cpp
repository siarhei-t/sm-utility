/**
 * @file
 *
 * @brief
 *
 * @author
 *
 */

#include "../inc/sm_com.hpp"

namespace sm
{

void Com::startReadData(std::uint8_t data[], const size_t amount)
{
    data_ready.store(false, std::memory_order_relaxed);
    platformReadData(data,amount);
}

void Com::startSendData(std::uint8_t data[], const size_t amount)
{
    platformSendData(data,amount);
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define, typename c, typename t>
void DataNode<amount_of_regs,amount_of_files,record_define,c,t>::start()
{
    com.startReadData(buffer,server.getReceiveBufferSize());
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define, typename c, typename t>
void DataNode<amount_of_regs,amount_of_files,record_define,c,t>::loop()
{
    //start receiver timeout timer
    if(com.data_in_progress.load(std::memory_order_relaxed) && !timer.isStarted())
    {
        timer.start();
    }
    // data process
    if(com.data_ready.load(std::memory_order_relaxed))
    {
        com.data_ready.store(false, std::memory_order_relaxed);
        last_error = server.serverTask(buffer, server.getReceiveBufferSize());
        com.startSendData(buffer,server.getTransmitBufferSize());
        com.startReadData(buffer,server.getReceiveBufferSize());
    }
    // port flush in case of timeout
    if(timer.done.load(std::memory_order_relaxed) && com.data_in_progress.load(std::memory_order_relaxed))
    {
        com.flush();
        timer.stop();
        com.startReadData(buffer,server.getReceiveBufferSize());
    }
}

} // namespace sm
