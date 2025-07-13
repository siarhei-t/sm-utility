/**
 * @file sm_server.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_SERVER_HPP
#define SM_SERVER_HPP

#include "../../common/sm_modbus.hpp"
#include "sm_resources.hpp"
#include <cstddef>
#include <cstdint>

namespace sm
{

enum class ServerExceptions
{
    no_error,
    address_not_recognized,
    bad_crc,
    function_exception
};

class ModbusServer
{
public:
    ModbusServer(std::uint8_t address, std::uint8_t record_size) : address(address), server_resources(record_size)
    {
        rx_length = modbus::address_size + modbus::min_pdu_with_data_size + modbus::crc_size;
    }
    ServerExceptions serverTask(std::uint8_t* data, const std::uint8_t length);
    std::uint8_t getReceiveBufferSize() const { return rx_length; }
    std::uint8_t getTransmitBufferSize() const { return tx_length; }

private:
    const std::uint8_t address;
    std::uint8_t tx_length = 0;
    std::uint8_t rx_length = 0;
    ServerResources server_resources;

    modbus::Exceptions writeRegister(std::uint8_t* data);
    modbus::Exceptions readRegister(std::uint8_t* data, std::uint8_t& length);
    modbus::Exceptions writeFile(std::uint8_t* data);
    modbus::Exceptions readFile(std::uint8_t* data, std::uint8_t& length);
    void generateException(std::uint8_t* data, const modbus::Exceptions exception);
    static std::uint16_t crc16(const std::uint8_t* data, const std::uint16_t length);
};

} // namespace sm

#endif // SM_SERVER_HPP
