/**
 * @file sm_resources.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_RESOURCES_HPP
#define SM_RESOURCES_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include "../../common/sm_modbus.hpp"

namespace sm
{

struct FileService
{
    FileService() = default;
    FileService(std::uint16_t file_id, std::uint16_t record_id, std::uint16_t length) : file_id(file_id), record_id(record_id), length(length) {}
    std::uint16_t file_id = 0;   // file id in modbus addressing model
    std::uint16_t record_id = 0; // record id in modbus addressing model
    std::uint16_t length = 0;    // received length from client in half words
};

struct FileControl
{
    size_t index = 0;                 // file index in files array in chip memory
    std::uint8_t* p_record = nullptr; // pointer to record
    std::uint8_t length = 0;          // actual record length in bytes
};

struct FileInfo
{
    FileInfo() = default;
    FileInfo(bool is_read, bool is_write, std::uint32_t size, std::uint8_t* p_data) : property_read(is_read), property_write(is_write), size(size), p_data(p_data) {}
    bool property_read = false;     // is file readable
    bool property_write = false;    // is file writable
    std::uint32_t size = 0;         // file size in bytes
    std::uint8_t* p_data = nullptr; // pointer to file data
};

struct RegisterInfo
{
    RegisterInfo() = default;
    RegisterInfo(bool is_write, bool is_read, std::uint16_t value) : property_read(is_read), property_write(is_write), value(value) {}
    bool property_read = false;  // is register readable
    bool property_write = false; // is register writable
    std::uint16_t value = 0;     // register value
};

template<size_t amount_of_regs, size_t amount_of_files> class ServerResources
{
public:
    bool writeRegister(const std::uint16_t address, const std::uint16_t value)
    {
        (void)(address);
        (void)(value);
        return true;
    }

    bool readRegister(const std::uint16_t address, const std::uint16_t quantity, std::uint8_t data[], std::uint8_t& length)
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
                    insertHalfWord(&data[counter], registers[offset_address + i].value);
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
    
    bool writeFile(const FileService& service, const std::uint8_t data[])
    {
        (void)(service);
        (void)(data);
        return true;
    }

    bool readFile(const FileService& service, std::uint8_t data[], std::uint8_t& length)
    {
        (void)(service);
        (void)(data);
        (void)(length);
        return true;
    }

    std::uint16_t extractHalfWord(const std::uint8_t data[])
    {
        std::uint16_t half_word = data[1];
        half_word |= static_cast<std::uint16_t>(data[0]) << 8;
        return half_word;
    }

    void insertHalfWord(std::uint8_t data[], const std::uint16_t half_word)
    {
        data[0] = static_cast<std::uint8_t>((half_word >> 8));
        data[1] = static_cast<std::uint8_t>(half_word);
    }

    void setBufferSize(const std::uint8_t new_size){ buffer_size = new_size; }
    
    std::uint8_t getBufferSize() const { return buffer_size; }

private:
    std::uint8_t buffer_size = 0;
    std::array<RegisterInfo, amount_of_regs> registers;
    std::array<FileInfo, amount_of_files> files;
};

} // namespace sm

#endif // SM_RESOURCES_HPP
