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

#include <cstddef>
#include <cstdint>
#include "sm_resources.hpp"
#include "../../common/sm_modbus.hpp"

namespace sm
{

enum class ServerExceptions
{
    no_error,
    address_not_recognized,
    bad_crc,
    function_exception
};

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define> class ModbusServer
{
public:
    ModbusServer(std::uint8_t address) : address(address), record_size(record_define)
    {
        setBufferSize(modbus::min_pdu_with_data_size);
    }

    ServerExceptions serverTask(std::uint8_t data[], const std::uint8_t length);

    std::uint8_t getReceiveBufferSize() const { return server_resources.getBufferSize(); }

    std::uint8_t getTransmitBufferSize() const { return tramsmit_length; }

private:
    const std::uint8_t address;
    const std::uint8_t record_size;
    std::uint8_t tramsmit_length = 0;

    ServerResources<amount_of_regs, amount_of_files> server_resources;

    void setBufferSize(const std::uint8_t new_size){ server_resources.setBufferSize(new_size); }

    modbus::Exceptions writeRegister(std::uint8_t data[]);

    modbus::Exceptions readRegister(std::uint8_t data[], std::uint8_t& length);

    modbus::Exceptions writeFile(std::uint8_t data[]);

    modbus::Exceptions readFile(std::uint8_t data[], std::uint8_t& length);

};

} // namespace sm

#endif // SM_SERVER_HPP
