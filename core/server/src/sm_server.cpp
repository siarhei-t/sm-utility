/**
 * @file
 *
 * @brief
 *
 * @author
 *
 */

#include "../inc/sm_server.hpp"
#include <cstddef>
#include <cstring>

namespace sm
{

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define> 
std::uint16_t ModbusServer<amount_of_regs, amount_of_files, record_define>::extractHalfWord(const std::uint8_t data[])
{
    std::uint16_t half_word = data[1];
    half_word |= static_cast<std::uint16_t>(data[0]) << 8;
    return half_word;
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define> 
void ModbusServer<amount_of_regs, amount_of_files, record_define>::insertHalfWord(std::uint8_t data[], const std::uint16_t half_word)
{
    data[0] = static_cast<std::uint8_t>((half_word >> 8));
    data[1] = static_cast<std::uint8_t>(half_word);
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define> 
ServerExceptions ModbusServer<amount_of_regs, amount_of_files, record_define>::serverTask(std::uint8_t data[], const std::uint8_t length)
{
    auto lambda_crc16 = [](const std::uint8_t data[], const std::uint16_t length)
    {
        std::uint16_t crc = 0xFFFF;
        std::uint16_t data_counter = length;
        std::uint16_t mem_counter = 0;

        for (; data_counter > 0; --data_counter)
        {
            std::uint8_t tmp = data[mem_counter] ^ crc;
            ++mem_counter;
            crc >>= 8;
            crc ^= modbus::crc16_table[tmp];
        }
        return crc;
    };

    auto lambda_generate_exception = [lambda_crc16](std::uint8_t data[], const modbus::Exceptions exception)
    {
        data[1] |= modbus::function_error_mask;
        data[2] = static_cast<std::uint8_t>(exception);
        return lambda_crc16(data, modbus::exception_pdu_size);
    };

    std::uint8_t received_address = data[0];
    std::uint8_t received_function = data[1];

    if (address != received_address)
    {
        return ServerExceptions::address_not_recognized;
    }
    std::uint16_t actual_crc = lambda_crc16(data, length - modbus::crc_size);
    std::uint16_t received_crc = data[length - modbus::crc_size];
    received_crc |= data[length - modbus::crc_size + 1];
    if (received_crc != actual_crc)
    {
        lambda_generate_exception(data, modbus::Exceptions::exception_3);
        return ServerExceptions::bad_crc;
    }

    modbus::Exceptions exception = modbus::Exceptions::no_exception;
    std::uint8_t generated_length = 0;
    size_t required_offset = modbus::address_size + modbus::function_size;
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
            lambda_generate_exception(data, modbus::Exceptions::exception_1);
            return ServerExceptions::function_exception;
    }
    if (exception != modbus::Exceptions::no_exception)
    {
        lambda_generate_exception(data, exception);
        return ServerExceptions::function_exception;
    }
    else
    {
        if (generated_length != 0)
        {
            std::uint16_t new_crc = lambda_crc16(data, required_offset + generated_length);
            data[required_offset + generated_length] = static_cast<std::uint8_t>(new_crc & 0xFF);
            data[required_offset + generated_length + 1] = static_cast<std::uint8_t>((new_crc & 0xFF00) >> 8);
        }
        return ServerExceptions::no_error;
    }
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define> 
modbus::Exceptions ModbusServer<amount_of_regs, amount_of_files, record_define>::writeRegister(std::uint8_t data[])
{
    std::uint16_t address = extractHalfWord(data);
    std::uint16_t value = extractHalfWord(data + sizeof(std::uint16_t));

    if (server_resources.writeRegister(address, value))
    {
        return modbus::Exceptions::no_exception;
    }
    else
    {
        return modbus::Exceptions::exception_4;
    }
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define> 
modbus::Exceptions ModbusServer<amount_of_regs, amount_of_files, record_define>::readRegister(std::uint8_t data[], std::uint8_t& length)
{
    std::uint16_t address = extractHalfWord(data);
    std::uint16_t quantity = extractHalfWord(data + sizeof(std::uint16_t));

    if ((quantity < modbus::min_amount_of_regs) && (quantity > modbus::max_amount_of_regs))
    {
        return modbus::Exceptions::exception_3;
    }
    else
    {
        if (server_resources.readRegister(address, quantity,length))
        {
            return modbus::Exceptions::no_exception;
        }
        else
        {
            return modbus::Exceptions::exception_2;
        }
    }
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define> 
modbus::Exceptions ModbusServer<amount_of_regs, amount_of_files, record_define>::writeFile(std::uint8_t data[])
{
    std::uint8_t byte_counter = data[0];
    std::uint8_t reference_type = data[1];
    FileService file_service(extractHalfWord(data + sizeof(std::uint16_t)), 
                             extractHalfWord(data + (sizeof(std::uint16_t) * 2)), 
                             extractHalfWord(data + (sizeof(std::uint16_t) * 3)));

    if ((reference_type != modbus::rw_file_reference) || (byte_counter < modbus::min_rw_file_byte_counter) || (byte_counter > modbus::max_rw_file_byte_counter))
    {
        return modbus::Exceptions::exception_3;
    }
    else
    {
        if (server_resources.writeFile(file_service, data + (sizeof(std::uint16_t) * 4)))
        {
            return modbus::Exceptions::no_exception;
        }
        else
        {
            return modbus::Exceptions::exception_2;
        }
    }
}

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define> 
modbus::Exceptions ModbusServer<amount_of_regs, amount_of_files, record_define>::readFile(std::uint8_t data[], std::uint8_t& length)
{
    std::uint8_t byte_counter = data[0];
    std::uint8_t reference_type = data[1];

    FileService file_service(extractHalfWord(data + sizeof(std::uint16_t)), 
                             extractHalfWord(data + (sizeof(std::uint16_t) * 2)), 
                             extractHalfWord(data + (sizeof(std::uint16_t) * 3)));

    if ((reference_type != modbus::rw_file_reference) || (byte_counter < modbus::min_rw_file_byte_counter) || (byte_counter > modbus::max_rw_file_byte_counter))
    {
        return modbus::Exceptions::exception_3;
    }
    else
    {
        if (server_resources.readFile(file_service, length))
        {
            return modbus::Exceptions::no_exception;
        }
        else
        {
            return modbus::Exceptions::exception_2;
        }
    }
}

/*
bool ServerResources::writeRegister(const std::uint16_t address, const std::uint16_t value) const
{
    (void)(address);
    (void)(value);
    return true;
}

bool ServerResources::readRegister(ServerCallback& server_callback, const std::uint16_t address, const std::uint16_t quantity) const
{
    const std::uint16_t offset_address = address - modbus::holding_regs_offset;
    if ((offset_address + quantity) <= static_cast<std::uint16_t>(ServerRegisters::count))
    {
        server_callback.data[0] = static_cast<std::uint8_t>((quantity * 2));
        int counter = 1;
        for (int i = 0; i < quantity; ++i)
        {
            ModbusServer::insertHalfWord(&server_callback.data[counter], registers[offset_address + i].value);
            counter += 2;
        }
        server_callback.length = (quantity * 2) + 1;
        return true;
    }
    else
    {
        return false;
    }
}


bool ModbusServer::writeFile(const FileService& service, const uint8_t data[]) const
{
    (void)(service);
    (void)(data);
    return true;
}

bool ModbusServer::readFile(const FileService& service) const
{
    (void)(service);
    return true;
}

*/

} // namespace sm
