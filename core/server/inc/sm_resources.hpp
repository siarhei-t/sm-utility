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
#include "../../common/sm_common.hpp"

namespace sm
{

constexpr int not_found = -1;

struct Data
{
    std::uint8_t* p_data = nullptr;
    std::uint32_t size = 0;
};

struct Attributes
{
    bool property_read = false;
    bool property_write = false;
};

struct FileControl
{
    size_t index = 0;                 // file index in files array in chip memory
    std::uint8_t* p_record = nullptr; // pointer to record
    std::uint8_t length = 0;          // actual record length in bytes
};

struct FileService
{
    FileService(std::uint16_t file_id, std::uint16_t record_id, std::uint16_t length) : file_id(file_id), record_id(record_id), length(length) {}
    std::uint16_t file_id = 0;   // file id in modbus addressing model
    std::uint16_t record_id = 0; // record id in modbus addressing model
    std::uint16_t length = 0;    // received length from client in half words
};

struct FileInfo
{
    FileInfo() = default;
    FileInfo(Attributes attributes, Data data) : attributes(attributes), data(data) {}
    Attributes attributes;
    Data data;
    void (*callback)(const FileInfo*) = nullptr; // callback on the end of write operation
};

struct RegisterInfo
{
    RegisterInfo() = default;
    RegisterInfo(Attributes attributes, std::uint16_t value) : attributes(attributes), value(value) {}
    Attributes attributes;
    std::uint16_t value = 0;                         // register value
    void (*callback)(const RegisterInfo*) = nullptr; // callback on the end of write operation
};

class ServerResources
{
public:
    ServerResources(std::uint8_t record_size) : record_size(record_size) {}
    bool writeRegister(const std::uint16_t address, const std::uint16_t value);
    bool readRegister(const std::uint16_t address, const std::uint16_t quantity, Data& data);
    bool writeFile(const FileService& service, const std::uint8_t* data);
    bool readFile(const FileService& service, Data& data);
    void setBufferSize(const std::uint8_t new_size){ buffer_size = new_size; }    
    std::uint8_t getBufferSize() const { return buffer_size; }
    static std::uint16_t extractHalfWord(const std::uint8_t* data);
    static void insertHalfWord(std::uint8_t* data, const std::uint16_t half_word);
private:
    std::uint8_t buffer_size = 0;
    const std::uint8_t record_size;
    std::array<RegisterInfo, RegisterDefinitions::getSize()> registers;
    std::array<FileInfo, FileDefinitions::getSize()> files;
};

} // namespace sm

#endif // SM_RESOURCES_HPP
