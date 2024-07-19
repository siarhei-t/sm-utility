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
constexpr std::uint8_t  file_record_size    = data_block_size * 6;
constexpr std::uint8_t  rw_reg_pdu_suze     = 5;
constexpr std::uint8_t  read_file_pdu_size  = 9;
constexpr std::uint8_t  service_min_size    = rw_reg_pdu_suze;
constexpr std::uint8_t  service_max_size    = read_file_pdu_size;
constexpr std::uint8_t  write_file_pdu_size = read_file_pdu_size + file_record_size;
////////////////////////////////////////////////////////////////////////////////

enum class ModbusMode
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
