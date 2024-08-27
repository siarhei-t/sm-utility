/**
 * @file
 *
 * @brief
 *
 * @author
 *
 */

#include "../inc/sm_resources.hpp"
#include "../../common/sm_modbus.hpp"
#include <cstddef>
#include <cstring>

namespace sm
{

template<size_t amount_of_regs, size_t amount_of_files> 
std::uint16_t ServerResources<amount_of_regs, amount_of_files>::extractHalfWord(const std::uint8_t data[])
{
    std::uint16_t half_word = data[1];
    half_word |= static_cast<std::uint16_t>(data[0]) << 8;
    return half_word;
}

template<size_t amount_of_regs, size_t amount_of_files> 
void ServerResources<amount_of_regs, amount_of_files>::insertHalfWord(std::uint8_t data[], const std::uint16_t half_word)
{
    data[0] = static_cast<std::uint8_t>((half_word >> 8));
    data[1] = static_cast<std::uint8_t>(half_word);
}

template<size_t amount_of_regs, size_t amount_of_files>
bool ServerResources<amount_of_regs,amount_of_files>::writeRegister(const std::uint16_t address, const std::uint16_t value)
{
    (void)(address);
    (void)(value);
    return true;
}

template<size_t amount_of_regs, size_t amount_of_files>
bool ServerResources<amount_of_regs,amount_of_files>::readRegister(const std::uint16_t address, const std::uint16_t quantity, std::uint8_t data[], std::uint8_t& length)
{
    const std::uint16_t offset_address = address - modbus::holding_regs_offset;
    if ((offset_address + quantity) <= registers.size())
    {
        data[0] = static_cast<std::uint8_t>((quantity * 2));
        auto counter = 1;
        for (int i = 0; i < quantity; ++i)
        {
            
            if(registers[offset_address + i].property_read)
            {
                insertHalfWord(data[counter], registers[offset_address + i].value);
                counter += 2;
            }
            else
            {
                return false;
            }
        }
        length =  data[0] + 1;
        return true;
    }
    else
    {
        return false;
    }
}

template<size_t amount_of_regs, size_t amount_of_files>
bool ServerResources<amount_of_regs,amount_of_files>::writeFile(const FileService& service, const uint8_t data[])
{
    (void)(service);
    (void)(data);
    return true;
}

template<size_t amount_of_regs, size_t amount_of_files>
bool ServerResources<amount_of_regs,amount_of_files>::readFile(const FileService& service, std::uint8_t data[], std::uint8_t& length)
{
    (void)(service);
    (void)(data);
    (void)(length);
    return true;
}


} // namespace sm
