/**
 * @file sm_modbus.hpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#ifndef SM_MESSAGE_H
#define SM_MESSAGE_H

#include <cstdint>
#include <vector>

namespace modbus
{
enum class ModbusMode
{
    pdu_only,
    rtu
};

class ModbusMessage
{
public:
    ModbusMessage(ModbusMode mode) : mode(mode) {}

    ModbusMode getMode() const { return mode; };

    void setMode(const ModbusMode new_mode) { mode = new_mode; };

    void msgCustom(std::vector<std::uint8_t>& buffer, const std::uint8_t func, const std::vector<std::uint8_t>& data, const std::uint8_t addr = 0);

    void msgWriteFileRecord(std::vector<std::uint8_t>& buffer, const std::uint16_t file_id, const std::uint16_t record_id,
                            const std::vector<std::uint8_t>& record_data, const std::uint8_t addr = 0);

    void msgReadFileRecord(std::vector<std::uint8_t>& buffer, const std::uint16_t file_id, const std::uint16_t record_id, const std::uint16_t length,
                           const std::uint8_t addr = 0);

    void msgWriteRegister(std::vector<std::uint8_t>& buffer, const std::uint16_t reg, const std::uint16_t value, const std::uint8_t addr = 0);

    void msgReadRegisters(std::vector<std::uint8_t>& buffer, const std::uint16_t reg, const std::uint16_t quantity, const std::uint8_t addr = 0);

    bool isChecksumValid(const std::vector<std::uint8_t>& data) const;

    bool extractData(const std::vector<std::uint8_t>& buffer, std::vector<std::uint8_t>& pdu) const;

    std::uint8_t getRequriedLength() const;

private:
    ModbusMode mode = ModbusMode::pdu_only;

    void createMessage(std::vector<std::uint8_t>& buffer, const std::uint8_t func, const std::vector<std::uint8_t>& data, const std::uint8_t addr = 0);

    std::uint16_t crc16(const std::vector<std::uint8_t>& data) const;
};

} // namespace modbus

#endif // SM_MESSAGE_H
