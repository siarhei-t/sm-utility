/**
 * @file sm_modbus.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sm_message.hpp"
#include "../../common/sm_log.hpp"
#include "../../common/sm_modbus.hpp"

namespace
{

void insertHalfWord(std::vector<std::uint8_t>& arr, const std::uint16_t value)
{
    arr.push_back((value >> 8) & 0xFF);
    arr.push_back(value & 0xFF);
}

} // namespace

namespace modbus
{

void ModbusMessage::msgCustom(std::vector<std::uint8_t>& buffer, const std::uint8_t func, const std::vector<std::uint8_t>& data, const std::uint8_t addr)
{
    createMessage(buffer, func, data, addr);
}

void ModbusMessage::msgWriteFileRecord(std::vector<std::uint8_t>& buffer, const std::uint16_t file_id, const std::uint16_t record_id,
                                       const std::vector<std::uint8_t>& record_data, const std::uint8_t addr)
{
    const std::uint8_t rec_data_length = record_data.size() + min_rw_file_byte_counter;
    const std::uint16_t record_length = record_data.size() / 2; // record splitted into half words
    std::vector<std::uint8_t> record;

    record.insert(record.end(), {rec_data_length, rw_file_reference});
    insertHalfWord(record, file_id);
    insertHalfWord(record, record_id);
    insertHalfWord(record, record_length);
    record.insert(record.end(), record_data.begin(), record_data.end());
    createMessage(buffer, static_cast<std::uint8_t>(FunctionCodes::write_file), record, addr);
}

void ModbusMessage::msgReadFileRecord(std::vector<std::uint8_t>& buffer, const std::uint16_t file_id, const std::uint16_t record_id, const std::uint16_t length,
                                      const std::uint8_t addr)
{
    std::vector<uint8_t> record;
    // 7 bytes in this message (support for reading only one record per message)
    record.insert(record.end(), {min_rw_file_byte_counter, rw_file_reference});
    insertHalfWord(record, file_id);
    insertHalfWord(record, record_id);
    insertHalfWord(record, length);
    createMessage(buffer, static_cast<std::uint8_t>(FunctionCodes::read_file), record, addr);
}

void ModbusMessage::msgWriteRegister(std::vector<std::uint8_t>& buffer, const std::uint16_t reg, const std::uint16_t value, const std::uint8_t addr)
{
    std::vector<uint8_t> data;
    insertHalfWord(data, reg);
    insertHalfWord(data, value);
    createMessage(buffer, static_cast<std::uint8_t>(FunctionCodes::write_reg), data, addr);
}

void ModbusMessage::msgReadRegisters(std::vector<std::uint8_t>& buffer, const std::uint16_t reg, const std::uint16_t quantity, const std::uint8_t addr)
{
    std::vector<std::uint8_t> data;
    insertHalfWord(data, reg);
    insertHalfWord(data, quantity);
    createMessage(buffer, static_cast<std::uint8_t>(FunctionCodes::read_regs), data, addr);
}

bool ModbusMessage::isChecksumValid(const std::vector<std::uint8_t>& data) const
{
    if (data.size() < min_pdu_with_data_size)
    {
        return false;
    }
    else
    {
        std::uint16_t crc_lo = data[data.size() - crc_size];
        std::uint16_t crc_hi = data[data.size() - crc_size + 1];
        std::uint16_t rec_crc = (crc_hi << 8) | crc_lo;
        std::uint16_t actual_crc = modbus::crc16(data.data(), static_cast<std::uint16_t>(data.size()) - crc_size);
        if (actual_crc == rec_crc)
        {
            return true;
        }
        else
        {
            LOG_INFO("error, expected crc : 0x%x , actual crc : 0x%x \n", rec_crc, actual_crc);
            return false;
        }
    }
}

bool ModbusMessage::extractData(const std::vector<std::uint8_t>& data, std::vector<std::uint8_t>& message) const
{
    message.clear();
    if (data.size() < min_pdu_with_data_size)
    {
        return false;
    }
    else
    {
        message.insert(message.end(), data.begin() + address_size, data.end() - crc_size);
        return true;
    }
}

std::uint8_t ModbusMessage::getRequiredLength() const
{
    std::uint8_t length;
    switch (mode)
    {
        case ModbusMode::rtu:
            length = rtu_adu_size;
            break;

        default:
            length = 0;
            break;
    }
    return length;
}

void ModbusMessage::createMessage(std::vector<std::uint8_t>& buffer, const std::uint8_t func, const std::vector<std::uint8_t>& data, const std::uint8_t addr)
{
    buffer.clear();
    if (mode == ModbusMode::rtu)
    {
        buffer.insert(buffer.end(), {addr});
    }
    buffer.insert(buffer.end(), {func});
    buffer.insert(buffer.end(), data.begin(), data.end());
    if (mode == ModbusMode::rtu)
    {
        uint16_t crc = modbus::crc16(buffer.data(), static_cast<std::uint16_t>(buffer.size()));
        buffer.push_back(crc & 0xFF);
        buffer.push_back((crc >> 8) & 0xFF);
    }
}

} // namespace modbus
