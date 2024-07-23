/**
 * @file sm_modbus.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_MODBUS_HPP
#define SM_MODBUS_HPP

#include <cstdint>

namespace modbus
{
////////////////////////////////MODBUS CONSTANTS////////////////////////////////
constexpr int crc_size           = 2;
constexpr int address_size       = 1;
constexpr int function_size      = 1;
constexpr int rtu_start_size     = 4;
constexpr int rtu_stop_size      = 4;
constexpr int ascii_start_size   = 1;
constexpr int ascii_stop_size    = 2;
constexpr int rtu_msg_edge       = (rtu_start_size + rtu_stop_size);
constexpr int ascii_msg_edge     = (ascii_start_size + ascii_stop_size);
constexpr int rtu_adu_size       = (rtu_msg_edge + crc_size + address_size);
constexpr int ascii_adu_size     = (ascii_msg_edge + crc_size + address_size);
constexpr int max_num_of_records = 10000;
constexpr std::uint16_t holding_regs_offset = 0x9C40;
constexpr std::uint16_t files_offset = 0x0001;
constexpr std::uint8_t  function_error_mask = 0x80;
constexpr std::uint8_t  max_adu_size = 253;
constexpr std::uint8_t  min_amount_of_regs = 1;
constexpr std::uint8_t  max_amount_of_regs = 125;
constexpr std::uint8_t  rw_file_reference = 6;
constexpr std::uint8_t  min_rw_file_byte_counter = 7;
constexpr std::uint8_t  max_rw_file_byte_counter = 245;
////////////////////////////////////////////////////////////////////////////////
// The size of this block must be a multiple of the encryption block
constexpr std::uint8_t  data_block_size     = 32;
// The size of this block together with the technical data should not exceed max_adu_size
constexpr std::uint8_t  exception_pdu_size  = 3;
constexpr std::uint8_t  file_record_size    = data_block_size * 6;
constexpr std::uint8_t  rw_reg_pdu_suze     = function_size + 4;
constexpr std::uint8_t  read_file_pdu_size  = function_size + 8;
constexpr std::uint8_t  service_min_size    = rw_reg_pdu_suze;
constexpr std::uint8_t  service_max_size    = read_file_pdu_size;
constexpr std::uint8_t  write_file_pdu_size = read_file_pdu_size + file_record_size;
constexpr std::uint8_t  max_pdu_size        = write_file_pdu_size;
//table for CRC16 with 0xA001 poly
constexpr std::uint16_t crc16_table[256] = 
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
////////////////////////////////////////////////////////////////////////////////

enum class Mode
{
    rtu,
    ascii
};

// General Modbus function codes, for reference see https://modbus.org/ 
enum class FunctionCodes
{
    read_regs  = 0x3,  // read holding registers
    write_reg  = 0x6,  // write single register
    read_file  = 0x14, // read file records
    write_file = 0x15, // write file records
    undefined  = 0xFF, // illegal function code
};

// General Modbus exception codes, for reference see https://modbus.org/ 
enum class Exceptions 
{
    no_exception = 0x0,
    exception_1  = 0x1,
    exception_2  = 0x2,
    exception_3  = 0x3,
    exception_4  = 0x4
};

} // namespace modbus

#endif // SM_MODBUS_HPP
