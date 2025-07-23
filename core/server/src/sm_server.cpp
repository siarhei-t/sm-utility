/**
 * @file sm_server.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sm_server.hpp"
#include <cstdio>

namespace sm
{

constexpr int required_offset = modbus::address_size + modbus::function_size;

ServerExceptions ModbusServer::serverTask(std::uint8_t* data, const std::uint8_t length)
{
    // recend what we have by default
    tx_length = length;

    std::uint8_t received_address = data[0];
    std::uint8_t received_function = data[1];

    std::printf("received message : \n");
    for (int i = 0; i < length; ++i)
    {
        std::printf("0x%x ", data[i]);
    }
    std::printf("\n");
    std::printf("received address : %d , function : %d  \n", received_address, received_function);
    if (address != received_address)
    {
        return ServerExceptions::address_not_recognized;
    }
    std::uint16_t actual_crc = modbus::crc16(data, length - modbus::crc_size);
    std::uint16_t received_crc = extract_half_word(data + (length - modbus::crc_size));
    std::printf("expected crc : 0x%x , actual crc : 0x%x \n", received_crc, actual_crc);
    if (received_crc != actual_crc)
    {
        std::printf("bad crc in received request! \n");

        generateException(data, modbus::Exceptions::exception_3);
        return ServerExceptions::bad_crc;
    }

    modbus::Exceptions exception = modbus::Exceptions::no_exception;
    std::uint8_t generated_length = 0;
    switch (received_function)
    {
        case static_cast<std::uint8_t>(modbus::FunctionCodes::write_reg):
            // we will resend the same data that we already have in buffer
            exception = writeRegister(data + required_offset);
            break;

        case static_cast<std::uint8_t>(modbus::FunctionCodes::read_regs):
            exception = readRegister(data + required_offset, generated_length);
            break;

        case static_cast<std::uint8_t>(modbus::FunctionCodes::write_file):
            exception = writeFile(data + required_offset);
            break;

        case static_cast<std::uint8_t>(modbus::FunctionCodes::read_file):
            exception = readFile(data + required_offset, generated_length);
            break;

        default:
            std::printf("unsupported function passed! \n");
            generateException(data, modbus::Exceptions::exception_1);
            return ServerExceptions::function_exception;
    }
    if (exception != modbus::Exceptions::no_exception)
    {
        generateException(data, exception);
        return ServerExceptions::function_exception;
    }
    else
    {
        if (generated_length != 0)
        {
            std::uint16_t new_crc = modbus::crc16(data, required_offset + generated_length);
            insert_half_word(data + required_offset + generated_length, new_crc);
            tx_length = required_offset + generated_length + modbus::crc_size;
        }
        return ServerExceptions::no_error;
    }
}

modbus::Exceptions ModbusServer::writeRegister(std::uint8_t* data)
{
    std::uint16_t address = extract_half_word(data);
    std::uint16_t value = extract_half_word(data + sizeof(std::uint16_t));

    if (server_resources->writeRegister(address, value))
    {
        return modbus::Exceptions::no_exception;
    }
    else
    {
        return modbus::Exceptions::exception_4;
    }
}

modbus::Exceptions ModbusServer::readRegister(std::uint8_t* data, std::uint8_t& length)
{
    std::uint16_t address = extract_half_word(data);
    std::uint16_t quantity = extract_half_word(data + sizeof(std::uint16_t));

    if ((quantity < modbus::min_amount_of_regs) && (quantity > modbus::max_amount_of_regs))
    {
        return modbus::Exceptions::exception_3;
    }
    else
    {
        if (server_resources->readRegister(address, quantity, data, length))
        {
            return modbus::Exceptions::no_exception;
        }
        else
        {
            return modbus::Exceptions::exception_2;
        }
    }
}

modbus::Exceptions ModbusServer::writeFile(std::uint8_t* data)
{
    std::uint8_t byte_counter = data[0];
    std::uint8_t reference_type = data[1];
    FileService file_service(extract_half_word(data + sizeof(std::uint16_t)), extract_half_word(data + (sizeof(std::uint16_t) * 2)),
                             extract_half_word(data + (sizeof(std::uint16_t) * 3)));

    if ((reference_type != modbus::rw_file_reference) || (byte_counter < modbus::min_rw_file_byte_counter) || (byte_counter > modbus::max_rw_file_byte_counter))
    {
        return modbus::Exceptions::exception_3;
    }
    else
    {
        if (server_resources->writeFile(file_service, data + (sizeof(std::uint16_t) * 4)))
        {
            return modbus::Exceptions::no_exception;
        }
        else
        {
            return modbus::Exceptions::exception_2;
        }
    }
}

modbus::Exceptions ModbusServer::readFile(std::uint8_t* data, std::uint8_t& length)
{
    std::uint8_t byte_counter = data[0];
    std::uint8_t reference_type = data[1];

    FileService file_service(extract_half_word(data + sizeof(std::uint16_t)), extract_half_word(data + (sizeof(std::uint16_t) * 2)),
                             extract_half_word(data + (sizeof(std::uint16_t) * 3)));

    if ((reference_type != modbus::rw_file_reference) || (byte_counter < modbus::min_rw_file_byte_counter) || (byte_counter > modbus::max_rw_file_byte_counter))
    {
        return modbus::Exceptions::exception_3;
    }
    else
    {
        if (server_resources->readFile(file_service, data, length))
        {
            return modbus::Exceptions::no_exception;
        }
        else
        {
            return modbus::Exceptions::exception_2;
        }
    }
}

void ModbusServer::generateException(std::uint8_t* data, const modbus::Exceptions exception)
{
    data[1] |= modbus::function_error_mask;
    data[2] = static_cast<std::uint8_t>(exception);
    std::uint16_t crc = modbus::crc16(data, modbus::exception_pdu_size + modbus::address_size);
    insert_half_word(data + modbus::address_size + modbus::exception_pdu_size, crc);
    tx_length = modbus::address_size + modbus::exception_pdu_size + modbus::crc_size;
};

} // namespace sm
