/**
 * @file sm_modbus.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sm_message.hpp"
#include "../../common/sm_modbus.hpp"
namespace
{
constexpr std::uint8_t rtu_start_end[] = {0x00, 0x00, 0x00, 0x00};
constexpr std::uint8_t ascii_start[] = {0x3A};
constexpr std::uint8_t ascii_stop[] = {0x0D, 0x0A};

static void insertHalfWord(std::vector<std::uint8_t>& arr, const std::uint16_t value)
{
    arr.push_back((value >> 8) & 0xFF);
    arr.push_back(value & 0xFF);
}

struct Sizes
{
    int start_seq_size;
    int stop_seq_size;
    int adu_header_size;
};

Sizes get_sizes(modbus::ModbusMode mode)
{
    switch (mode)
    {

        case modbus::ModbusMode::ascii:
            return Sizes{modbus::ascii_start_size, modbus::ascii_stop_size, modbus::ascii_adu_size};

        case modbus::ModbusMode::rtu:
            return Sizes{modbus::rtu_start_size, modbus::rtu_stop_size, modbus::rtu_adu_size};
    }
    return {};
}
} // namespace

namespace modbus
{
std::vector<std::uint8_t>& ModbusClient::msgCustom(const std::uint8_t addr, const std::uint8_t func, const std::vector<std::uint8_t>& data)
{
    createMessage(addr, func, data);
    return buffer;
}

std::vector<std::uint8_t>& ModbusClient::msgWriteFileRecord(const std::uint8_t addr, const std::uint16_t file_id, const std::uint16_t record_id,
                                                            const std::vector<std::uint8_t>& record_data)
{
    const std::uint8_t rec_data_length = record_data.size() + 7; // 7 additional bytes for record data
    const std::uint16_t record_length = record_data.size() / 2;  // record splited into half words
    std::vector<std::uint8_t> record;

    record.insert(record.end(), {rec_data_length, 0x06U});
    insertHalfWord(record, file_id);
    insertHalfWord(record, record_id);
    insertHalfWord(record, record_length);
    record.insert(record.end(), record_data.begin(), record_data.end());
    createMessage(addr, static_cast<std::uint8_t>(FunctionCodes::write_file), record);
    return buffer;
}

std::vector<std::uint8_t>& ModbusClient::msgReadFileRecord(const std::uint8_t addr, const std::uint16_t file_id, const std::uint16_t record_id,
                                                           const std::uint16_t length)
{
    std::vector<uint8_t> record;
    record.insert(record.end(), {0x07, 0x06}); // 7 bytes in this message (support for reading
                                               // only one record per message)
    insertHalfWord(record, file_id);
    insertHalfWord(record, record_id);
    insertHalfWord(record, length);
    createMessage(addr, static_cast<std::uint8_t>(FunctionCodes::read_file), record);
    return buffer;
}

std::vector<std::uint8_t>& ModbusClient::msgWriteRegister(const std::uint8_t addr, const std::uint16_t reg, const std::uint16_t value)
{
    std::vector<uint8_t> data;
    insertHalfWord(data, reg);
    insertHalfWord(data, value);
    createMessage(addr, static_cast<std::uint8_t>(FunctionCodes::write_reg), data);
    return buffer;
}

std::vector<std::uint8_t>& ModbusClient::msgReadRegisters(const std::uint8_t addr, const std::uint16_t reg, const std::uint16_t quantity)
{
    std::vector<std::uint8_t> data;
    insertHalfWord(data, reg);
    insertHalfWord(data, quantity);
    createMessage(addr, static_cast<std::uint8_t>(FunctionCodes::read_regs), data);
    return buffer;
}

bool ModbusClient::isChecksumValid(const std::vector<std::uint8_t>& data)
{
    std::vector<std::uint8_t> message;
    Sizes sizes = get_sizes(mode);

    const int crc_idx = data.size() - sizes.stop_seq_size - crc_size;
    if (data.size() <= static_cast<size_t>(sizes.adu_header_size))
    {
        return false; // undefined data in vector
    }
    else
    {
        message.insert(message.end(), data.begin() + sizes.start_seq_size, data.end() - sizes.stop_seq_size - crc_size);
        std::uint16_t rec_crc = data[crc_idx];
        rec_crc = (rec_crc << 8) | data[crc_idx + 1];
        std::uint16_t actual_crc = crc16(message);
        if (actual_crc == rec_crc)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

void ModbusClient::extractData(const std::vector<std::uint8_t>& data, std::vector<std::uint8_t>& message)
{
    Sizes sizes = get_sizes(mode);
    message.insert(message.end(), data.begin() + sizes.start_seq_size, data.end() - sizes.stop_seq_size);
}

std::uint8_t ModbusClient::getRequriedLength() const
{
    std::uint8_t length = 0;
    switch (mode)
    {
        case ModbusMode::rtu:
            length = rtu_adu_size;
            break;

        case ModbusMode::ascii:
            length = ascii_adu_size;
            break;
    }
    return length;
}

void ModbusClient::createMessage(const std::uint8_t addr, const std::uint8_t func, const std::vector<std::uint8_t>& data)
{
    buffer.clear();
    // setup PDU
    buffer.insert(buffer.end(), {addr, func});
    buffer.insert(buffer.end(), data.begin(), data.end());
    uint16_t crc = crc16(buffer);
    insertHalfWord(buffer, crc);
    // setup ADU
    switch (mode)
    {
        case ModbusMode::rtu:
            buffer.insert(buffer.begin(), &rtu_start_end[0], &rtu_start_end[0] + sizeof(rtu_start_end));
            buffer.insert(buffer.end(), &rtu_start_end[0], &rtu_start_end[0] + sizeof(rtu_start_end));
            break;

        case ModbusMode::ascii:
            buffer.insert(buffer.begin(), &ascii_start[0], &ascii_start[0] + sizeof(ascii_start));
            buffer.insert(buffer.end(), &ascii_stop[0], &ascii_stop[0] + sizeof(ascii_stop));
            break;
    }
}

std::uint16_t ModbusClient::crc16(const std::vector<std::uint8_t>& data)
{
    const std::uint16_t ibm_poly = 0xA001U;
    std::uint16_t result = 0xFFFFU;

    auto ibm_byte{[](std::uint16_t crc, std::uint8_t data) -> std::uint16_t
                  {
                      const std::uint16_t table[2] = {0x0000, ibm_poly};
                      std::uint8_t xOr = 0;
                      crc ^= data;
                      for (std::uint8_t bit = 0; bit < 8; bit++)
                      {
                          xOr = crc & 0x01;
                          crc >>= 1;
                          crc ^= table[xOr];
                      }
                      return crc;
                  }};

    for (std::size_t i = 0; i < data.size(); ++i)
    {
        result = ibm_byte(result, data[i]);
    }
    return result;
}

} // namespace modbus
