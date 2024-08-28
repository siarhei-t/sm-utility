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
    bool writeRegister(const std::uint16_t address, const std::uint16_t value);

    bool readRegister(const std::uint16_t address, const std::uint16_t quantity, std::uint8_t data[], std::uint8_t& length);

    bool writeFile(const FileService& service, const std::uint8_t data[]);

    bool readFile(const FileService& service, std::uint8_t data[], std::uint8_t& length);
    
    std::uint16_t extractHalfWord(const std::uint8_t data[]);
    
    void insertHalfWord(std::uint8_t data[], const std::uint16_t half_word);

private:
    std::array<RegisterInfo, amount_of_regs> registers;
    std::array<FileInfo, amount_of_files> files;
};

} // namespace sm

#endif // SM_RESOURCES_HPP
