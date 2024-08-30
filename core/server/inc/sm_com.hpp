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

class Com
{
public:
    void readData(std::uint8_t data[], const size_t amount);
    void sendData(const std::uint8_t data[], const size_t amount);
};

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define, typename c = Com> class DataNode
{
public:
    DataNode(std::uint8_t address) : server(address) {}
    void loop(); 
    std::uint8_t* getBufferPtr() const;

private:
    std::uint8_t rx_length = 0;
    std::uint8_t tx_length = 0;
    ModbusServer<amount_of_regs,amount_of_files,record_define> server;
    c com;
};

} // namespace sm

#endif // SM_COM_HPP
