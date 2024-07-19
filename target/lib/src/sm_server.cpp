/**
 * @file 
 *
 * @brief
 *
 * @author 
 *
 */

#include <iostream>
#include <iterator>
#include "../inc/sm_server.hpp"

namespace sm
{

ServerExceptions ModbusServer::serverTask(std::uint8_t data[]) const
{
    modbus::Exceptions exception = modbus::Exceptions();
    std::uint8_t received_address = data[0];
    std::uint8_t received_function =  data[1];

    // (1) address check
    if(address != received_address)
    {
        return ServerExceptions::address_not_recognized;
    }
    // (2) crc check (in case of RTU)
    std::uint8_t data_length = getMessageLength(received_function);
    if(data_length == 0)
    {
        //add here error response generator
        return ServerExceptions::bad_crc;
    }
    uint16_t received_crc = extractHalfWord(&data[data_length]);
    std::uint16_t actual_crc = CRC16(data,data_length);
    
    if(received_crc != actual_crc)
    {
        //add here error response generator
        return ServerExceptions::bad_crc;
    }
    // (3) function execution
    switch (received_function) 
    {
        case static_cast<std::uint8_t>(modbus::FunctionCodes::read_regs):
            exception = readRegister(&data[2]);
            break;

        case static_cast<std::uint8_t>(modbus::FunctionCodes::write_reg):
            exception = writeRegister(&data[2]);
            break;

        case static_cast<std::uint8_t>(modbus::FunctionCodes::read_file):
            exception = readFile(&data[2]);
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
    constexpr std::uint8_t data_length  = modbus::rw_reg_pdu_suze;
    constexpr std::uint8_t index_crc = modbus::rw_reg_pdu_suze;
    constexpr std::uint8_t index_address = modbus::function_size;
    constexpr std::uint8_t index_value = index_address + 2;

    uint16_t received_crc = extractHalfWord(&data[index_crc]);
    uint16_t actual_crc  = CRC16(data,data_length);

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
    const std::uint8_t data_length  = modbus::rw_reg_pdu_suze;
    const std::uint8_t index_crc = modbus::rw_reg_pdu_suze;
    const std::uint8_t index_address = modbus::function_size;
    const std::uint8_t index_quantity = index_address + 2;

    uint16_t received_crc = extractHalfWord(&data[index_crc]);
    uint16_t actual_crc  = CRC16(data,data_length);
    if(received_crc == actual_crc)
    {
        uint16_t address  = extractHalfWord(&data[index_address]);
        uint16_t quantity = extractHalfWord(&data[index_quantity]);

        if((quantity < modbus::min_amount_of_regs) && (quantity > modbus::max_amount_of_regs))
        {
            return modbus::Exceptions::exception_3;
        }
        else
        {
            if(quantity <= static_cast<std::uint8_t>(ServerRegisters::count)) 
            {
                if(!readRegister(address,quantity))
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
    else
    {
        return modbus::Exceptions::exception_3;
    }
}

modbus::Exceptions ModbusServer::writeFile(std::uint8_t data[]) const
{
    constexpr std::uint8_t data_length  = modbus::write_file_pdu_size;
    constexpr std::uint8_t index_crc = modbus::write_file_pdu_size;

    std::uint16_t received_crc = extractHalfWord(&data[index_crc]);
    std::uint16_t actual_crc  = CRC16(data,data_length);
    if(received_crc == actual_crc)
    {
        FileService file_service(extractHalfWord(&data[modbus::function_size + 2]),
                                 extractHalfWord(&data[modbus::function_size + 4]),
                                 extractHalfWord(&data[modbus::function_size + 6]));
        
        uint8_t byte_counter = data[modbus::function_size];
        uint8_t reference_type = data[modbus::function_size + 1];
        
        if((reference_type != modbus::rw_file_reference) || 
           (byte_counter   <  modbus::min_rw_file_byte_counter) || 
           (byte_counter   >  modbus::max_rw_file_byte_counter) 
          )
        {
            return modbus::Exceptions::exception_3;
        }
        else
        {
            if(writeFile(file_service,&data[modbus::function_size + 8]))
            {
                return modbus::Exceptions::no_exception;
            }
            else
            {
                return modbus::Exceptions::exception_2;
            }
        }
    }
    else
    {
        return modbus::Exceptions::exception_3;
    }
}

modbus::Exceptions ModbusServer::readFile(std::uint8_t data[]) const
{
    const std::uint8_t data_length  = modbus::read_file_pdu_size;
    const std::uint8_t index_crc = modbus::read_file_pdu_size;
    
    std::uint16_t received_crc = extractHalfWord(&data[index_crc]);
    std::uint16_t actual_crc  = CRC16(data,data_length);
    
    if(received_crc == actual_crc)
    {
        FileService file_service(extractHalfWord(&data[modbus::function_size + 2]),
                                 extractHalfWord(&data[modbus::function_size + 4]),
                                 extractHalfWord(&data[modbus::function_size + 6]));

        uint8_t byte_counter = data[modbus::function_size];
        uint8_t reference_type = data[modbus::function_size + 1];
        
        if((reference_type != modbus::rw_file_reference) || 
           (byte_counter < modbus::min_rw_file_byte_counter) || 
           (byte_counter > modbus::max_rw_file_byte_counter) 
          )
        {
            return modbus::Exceptions::exception_3;
        }
        else
        {
            if(!readFile(file_service))
            {
                return modbus::Exceptions::exception_2;
            }
            else
            {
                return modbus::Exceptions::no_exception;
            }
        }
    }
    else
    {
        return modbus::Exceptions::exception_3;
    }
}

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

std::uint8_t ModbusServer::getMessageLength(const std::uint8_t function) const
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
}

std::uint16_t ModbusServer::extractHalfWord(const std::uint8_t data[]) const
{
     std::uint16_t half_word  = data[1];
                   half_word |= static_cast<std::uint16_t>(data[0]) << 8;
    return half_word;
}

std::uint16_t ModbusServer::CRC16(const std::uint8_t data[], const std::uint16_t length) const
{
   static const std::uint16_t table[256] = 
    {
    0X0000u, 0XC0C1u, 0XC181u, 0X0140u, 0XC301u, 0X03C0u, 0X0280u, 0XC241u,
    0XC601u, 0X06C0u, 0X0780u, 0XC741u, 0X0500u, 0XC5C1u, 0XC481u, 0X0440u,
    0XCC01u, 0X0CC0u, 0X0D80u, 0XCD41u, 0X0F00u, 0XCFC1u, 0XCE81u, 0X0E40u,
    0X0A00u, 0XCAC1u, 0XCB81u, 0X0B40u, 0XC901u, 0X09C0u, 0X0880u, 0XC841u,
    0XD801u, 0X18C0u, 0X1980u, 0XD941u, 0X1B00u, 0XDBC1u, 0XDA81u, 0X1A40u,
    0X1E00u, 0XDEC1u, 0XDF81u, 0X1F40u, 0XDD01u, 0X1DC0u, 0X1C80u, 0XDC41u,
    0X1400u, 0XD4C1u, 0XD581u, 0X1540u, 0XD701u, 0X17C0u, 0X1680u, 0XD641u,
    0XD201u, 0X12C0u, 0X1380u, 0XD341u, 0X1100u, 0XD1C1u, 0XD081u, 0X1040u,
    0XF001u, 0X30C0u, 0X3180u, 0XF141u, 0X3300u, 0XF3C1u, 0XF281u, 0X3240u,
    0X3600u, 0XF6C1u, 0XF781u, 0X3740u, 0XF501u, 0X35C0u, 0X3480u, 0XF441u,
    0X3C00u, 0XFCC1u, 0XFD81u, 0X3D40u, 0XFF01u, 0X3FC0u, 0X3E80u, 0XFE41u,
    0XFA01u, 0X3AC0u, 0X3B80u, 0XFB41u, 0X3900u, 0XF9C1u, 0XF881u, 0X3840u,
    0X2800u, 0XE8C1u, 0XE981u, 0X2940u, 0XEB01u, 0X2BC0u, 0X2A80u, 0XEA41u,
    0XEE01u, 0X2EC0u, 0X2F80u, 0XEF41u, 0X2D00u, 0XEDC1u, 0XEC81u, 0X2C40u,
    0XE401u, 0X24C0u, 0X2580u, 0XE541u, 0X2700u, 0XE7C1u, 0XE681u, 0X2640u,
    0X2200u, 0XE2C1u, 0XE381u, 0X2340u, 0XE101u, 0X21C0u, 0X2080u, 0XE041u,
    0XA001u, 0X60C0u, 0X6180u, 0XA141u, 0X6300u, 0XA3C1u, 0XA281u, 0X6240u,
    0X6600u, 0XA6C1u, 0XA781u, 0X6740u, 0XA501u, 0X65C0u, 0X6480u, 0XA441u,
    0X6C00u, 0XACC1u, 0XAD81u, 0X6D40u, 0XAF01u, 0X6FC0u, 0X6E80u, 0XAE41u,
    0XAA01u, 0X6AC0u, 0X6B80u, 0XAB41u, 0X6900u, 0XA9C1u, 0XA881u, 0X6840u, 
    0X7800u, 0XB8C1u, 0XB981u, 0X7940u, 0XBB01u, 0X7BC0u, 0X7A80u, 0XBA41u,
    0XBE01u, 0X7EC0u, 0X7F80u, 0XBF41u, 0X7D00u, 0XBDC1u, 0XBC81u, 0X7C40u,
    0XB401u, 0X74C0u, 0X7580u, 0XB541u, 0X7700u, 0XB7C1u, 0XB681u, 0X7640u,
    0X7200u, 0XB2C1u, 0XB381u, 0X7340u, 0XB101u, 0X71C0u, 0X7080u, 0XB041u,
    0X5000u, 0X90C1u, 0X9181u, 0X5140u, 0X9301u, 0X53C0u, 0X5280u, 0X9241u,
    0X9601u, 0X56C0u, 0X5780u, 0X9741u, 0X5500u, 0X95C1u, 0X9481u, 0X5440u,
    0X9C01u, 0X5CC0u, 0X5D80u, 0X9D41u, 0X5F00u, 0X9FC1u, 0X9E81u, 0X5E40u,
    0X5A00u, 0X9AC1u, 0X9B81u, 0X5B40u, 0X9901u, 0X59C0u, 0X5880u, 0X9841u,
    0X8801u, 0X48C0u, 0X4980u, 0X8941u, 0X4B00u, 0X8BC1u, 0X8A81u, 0X4A40u,
    0X4E00u, 0X8EC1u, 0X8F81u, 0X4F40u, 0X8D01u, 0X4DC0u, 0X4C80u, 0X8C41u,
    0X4400u, 0X84C1u, 0X8581u, 0X4540u, 0X8701u, 0X47C0u, 0X4680u, 0X8641u,
    0X8201u, 0X42C0u, 0X4380u, 0X8341u, 0X4100u, 0X81C1u, 0X8081u, 0X4040u 
    };

    std::uint16_t crc = 0xFFFF;
    std::uint16_t data_counter = length;
    std::uint16_t mem_counter  = 0;

    for ( ; data_counter > 0; --data_counter)
    {
        std::uint8_t tmp = data[mem_counter] ^ crc;
        ++mem_counter;
        crc >>= 8;
        crc ^= table[tmp];
    }
    return crc;
}

} // namespace modbus
