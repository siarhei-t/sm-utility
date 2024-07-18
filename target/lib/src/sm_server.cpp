/**
 * @file 
 *
 * @brief
 *
 * @author 
 *
 */

#include "../inc/sm_server.hpp"

namespace sm
{

ModbusServer::ModbusServer(modbus::ModbusMode mode) : mode(mode)
{
    switch (mode) 
    {
        case modbus::ModbusMode::ascii:
            server_config.msg_start_size = modbus::ascii_start_size;
            server_config.msg_stop_size = modbus::ascii_stop_size;
            break;

        case modbus::ModbusMode::rtu:
            server_config.msg_start_size = modbus::rtu_start_size;
            server_config.msg_stop_size = modbus::rtu_stop_size;
            break;
    }
    server_config.package_edge_size = server_config.msg_start_size + server_config.msg_stop_size;
    server_config.data_start_idx = server_config.msg_start_size + modbus::address_size + modbus::function_size;
    server_config.adu_requried_size = server_config.package_edge_size + modbus::crc_size + modbus::address_size;
    server_config.buffer_init_size = server_config.adu_requried_size + modbus::service_min_size;
}

std::uint16_t ModbusServer::extractHalfWord(const std::uint8_t data[])
{
     std::uint16_t half_word  = data[1];
                   half_word |= static_cast<std::uint16_t>(data[0]) << 8;
    return half_word;
}

std::uint16_t ModbusServer::CRC16(const std::uint8_t data[], const std::uint16_t length)
{
    return 0;
}

modbus::Exceptions ModbusServer::writeRegister(std::uint8_t data[]) const
{
    const std::uint8_t data_length  = modbus::rw_reg_pdu_suze + modbus::address_size;
    const std::uint8_t index_crc = modbus::rw_reg_pdu_suze + modbus::address_size + server_config.msg_start_size;
    const std::uint8_t index_address = server_config.data_start_idx;
    const std::uint8_t index_value = server_config.data_start_idx + 2;

    uint16_t received_crc = extractHalfWord(&data[index_crc]);
    uint16_t actual_crc  = CRC16(&data[server_config.msg_start_size],data_length);
    
    if(received_crc == actual_crc)
    {
        std::uint16_t address = extractHalfWord(&data[index_address]);
        std::uint16_t value   = extractHalfWord(&data[index_value]);
        if(writeRegister(address,value))
        {
            //success
            return modbus::Exceptions::no_exception;
        }
        else
        {
            return modbus::Exceptions::exception_4;
        }
    }
    else
    {
       return modbus::Exceptions::exception_3;
    }
}

modbus::Exceptions ModbusServer::readRegister(std::uint8_t data[]) const
{
    return modbus::Exceptions();
}

modbus::Exceptions ModbusServer::writeFile(std::uint8_t data[]) const
{
    return modbus::Exceptions();
}

modbus::Exceptions ModbusServer::readFile(std::uint8_t data[]) const
{
    return modbus::Exceptions();
}

bool ModbusServer::writeRegister(const std::uint16_t address, const std::uint16_t value) const
{
    return true;
}

bool ModbusServer::readRegister(const std::uint16_t address, const std::uint16_t quantity) const
{
    return true;
}

bool ModbusServer::writeFile(const FileService& service, const uint8_t data[]) const
{
    return true;
}

bool ModbusServer::readFile(const FileService& service) const
{
    return true;
}

} // namespace modbus
