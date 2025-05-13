/**
 * @file sm_resources.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sm_resources.hpp"
#include "../../common/sm_modbus.hpp"

namespace sm
{

bool ServerResources::writeRegister(const std::uint16_t address, const std::uint16_t value)
{
    if(address < modbus::holding_regs_offset) { return false; }
    const std::uint16_t offset_address = address - modbus::holding_regs_offset;
    if (offset_address > registers.size()) { return false; }
    if(registers[offset_address].attributes.property_write)
    {
        registers[offset_address].value = value;
        if(registers[offset_address].callback != nullptr)
        {
            registers[offset_address].callback(&registers[offset_address]);
        }
        return true;
    }
    else { return false; }
}

bool ServerResources::readRegister(const std::uint16_t address, const std::uint16_t quantity, std::uint8_t* data, std::uint8_t& size)
{
    if(address < modbus::holding_regs_offset) { return false; }
    const std::uint16_t offset_address = address - modbus::holding_regs_offset;
    if (offset_address > (registers.size() - quantity )) { return false; }
    data[0] = static_cast<std::uint8_t>((quantity * 2));
    int counter = 1;
    for (int i = 0; i < quantity; ++i)
    {
        if(registers[offset_address + i].attributes.property_read)
        {
            insertHalfWord(&data[counter], registers[offset_address + i].value);
            counter += 2;
        }
        else { return false; }
    }
    size =  data[0] + 1;
    return true;
}

bool ServerResources::writeFile(const FileService& service, const std::uint8_t* data)
{
    (void)(service);
    (void)(data);
    return true;
}

bool ServerResources::readFile(const FileService& service, std::uint8_t* data, std::uint8_t& size)
{
    (void)(service);
    (void)(data);
    (void)(size);
    return true;
}

std::uint16_t ServerResources::extractHalfWord(const std::uint8_t* data)
{
    std::uint16_t half_word = data[1];
    half_word |= static_cast<std::uint16_t>(data[0]) << 8;
    return half_word;
}

void ServerResources::insertHalfWord(std::uint8_t* data, const std::uint16_t half_word)
{
    data[0] = static_cast<std::uint8_t>((half_word >> 8));
    data[1] = static_cast<std::uint8_t>(half_word);
}

} // namespace sm
