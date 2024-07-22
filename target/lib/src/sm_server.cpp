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

ServerExceptions ModbusServer::serverTask(std::uint8_t data[]) const
{
    auto lambda_crc16 = [](const std::uint8_t data[], const std::uint16_t length)
    {
        std::uint16_t crc = 0xFFFF;
        std::uint16_t data_counter = length;
        std::uint16_t mem_counter  = 0;

        for ( ; data_counter > 0; --data_counter)
        {
            std::uint8_t tmp = data[mem_counter] ^ crc;
            ++mem_counter;
            crc >>= 8;
            crc ^= modbus::crc16_table[tmp];
        }
        return crc;
    };

    auto lambda_get_msg_length = [](const std::uint8_t function)
    {
        std::uint8_t length = 0;
        switch (function) 
        {
            case static_cast<std::uint8_t>(modbus::FunctionCodes::read_regs):
            case static_cast<std::uint8_t>(modbus::FunctionCodes::write_reg):
                length = modbus::read_file_pdu_size;
                break;
            
            case static_cast<std::uint8_t>(modbus::FunctionCodes::read_file):
                length = modbus::read_file_pdu_size;
                break;

            // support only for writing record with modbus::file_record_size
            case static_cast<std::uint8_t>(modbus::FunctionCodes::write_file):
                length = modbus::write_file_pdu_size;
                break;

            default:
                break;
        }
        return length;
    };

    modbus::Exceptions exception;
    std::uint8_t received_address = data[0];
    std::uint8_t received_function =  data[1];

    // (1) address check
    if(address != received_address)
    {
        return ServerExceptions::address_not_recognized;
    }
    // (2) crc check (in case of RTU)
    std::uint8_t data_length = lambda_get_msg_length(received_function);
    if(data_length == 0)
    {
        //add here error response generator
        return ServerExceptions::bad_crc;
    }
    std::uint16_t received_crc = extractHalfWord(&data[data_length]);
    std::uint16_t actual_crc = lambda_crc16(data,data_length);
    
    if(received_crc != actual_crc)
    {
        //add here error response generator
        return ServerExceptions::bad_crc;
    }
    // (3) function execution
    ServerCallback server_callback;
    switch (received_function)
    {
        case static_cast<std::uint8_t>(modbus::FunctionCodes::read_regs):
            exception = readRegister(&data[2],server_callback);
            break;

        case static_cast<std::uint8_t>(modbus::FunctionCodes::write_reg):
            exception = writeRegister(&data[2]);
            break;

        case static_cast<std::uint8_t>(modbus::FunctionCodes::read_file):
            exception = readFile(&data[2],server_callback);
            break;

        case static_cast<std::uint8_t>(modbus::FunctionCodes::write_file):
            exception = writeFile(&data[2]);
            break;
        
        default:
            //add here error responce generator
            return ServerExceptions::function_exception;
            break;
    }
    if(exception != modbus::Exceptions::no_exception)
    {
        //add here error responce generator
        return ServerExceptions::function_exception;
    }
    else
    {
        return ServerExceptions::no_error;
    }
}

modbus::Exceptions ModbusServer::writeRegister(std::uint8_t data[]) const
{
    std::uint16_t address = extractHalfWord(&data[0]);
    std::uint16_t value   = extractHalfWord(&data[2]);
    if(true)
    {
        //success
        return modbus::Exceptions::no_exception;
    }
    else
    {
        return modbus::Exceptions::exception_4;
    }
}

modbus::Exceptions ModbusServer::readRegister(std::uint8_t data[], ServerCallback& server_callback) const
{
    std::uint16_t address  = extractHalfWord(&data[0]);
    std::uint16_t quantity = extractHalfWord(&data[2]);

    if((quantity < modbus::min_amount_of_regs) && (quantity > modbus::max_amount_of_regs))
    {
        return modbus::Exceptions::exception_3;
    }
    else
    {
        if(quantity <= static_cast<std::uint8_t>(ServerRegisters::count)) 
        {
            if(!true)
            {
                return modbus::Exceptions::exception_2;
            }
            else
            {
                return modbus::Exceptions::no_exception;
            }
        }
        else
        {
            return modbus::Exceptions::exception_2;
        }
    }
}

modbus::Exceptions ModbusServer::writeFile(std::uint8_t data[]) const
{
    std::uint8_t byte_counter = data[0];
    std::uint8_t reference_type = data[1];
    FileService file_service(extractHalfWord(&data[2]),
                                extractHalfWord(&data[4]),
                                extractHalfWord(&data[6]));

    if((reference_type != modbus::rw_file_reference) || 
        (byte_counter   <  modbus::min_rw_file_byte_counter) || 
        (byte_counter   >  modbus::max_rw_file_byte_counter) 
        )
    {
        return modbus::Exceptions::exception_3;
    }
    else
    {
        if(true)
        {
            return modbus::Exceptions::no_exception;
        }
        else
        {
            return modbus::Exceptions::exception_2;
        }
    }
}

modbus::Exceptions ModbusServer::readFile(std::uint8_t data[], ServerCallback& server_callback) const
{
    std::uint8_t byte_counter = data[0];
    std::uint8_t reference_type = data[0];

    FileService file_service(extractHalfWord(&data[2]),
                                extractHalfWord(&data[4]),
                                extractHalfWord(&data[6]));
    
    if((reference_type != modbus::rw_file_reference) || 
        (byte_counter < modbus::min_rw_file_byte_counter) || 
        (byte_counter > modbus::max_rw_file_byte_counter) 
        )
    {
        return modbus::Exceptions::exception_3;
    }
    else
    {
        if(!true)
        {
            return modbus::Exceptions::exception_2;
        }
        else
        {
            return modbus::Exceptions::no_exception;
        }
    }

}

/*
bool ModbusServer::writeRegister(const std::uint16_t address, const std::uint16_t value) const
{
    (void)(address);
    (void)(value);
    return true;
}

bool ModbusServer::readRegister(const std::uint16_t address, const std::uint16_t quantity) const
{
    (void)(address);
    (void)(quantity);
    return true;
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

std::uint16_t ModbusServer::extractHalfWord(const std::uint8_t data[]) const
{
     std::uint16_t half_word  = data[1];
                   half_word |= static_cast<std::uint16_t>(data[0]) << 8;
    return half_word;
}

} // namespace modbus
