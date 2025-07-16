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

#include "../../common/sm_common.hpp"
#include "../../common/sm_modbus.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace sm
{

constexpr size_t not_found = -1;
constexpr std::uint8_t default_buffer_size = modbus::address_size + modbus::min_pdu_with_data_size + modbus::crc_size;
;
struct Attributes
{
    bool property_read = false;
    bool property_write = false;
    bool property_flash = false;
};

struct FileData
{
    std::uint8_t* p_data = nullptr;
    std::uint32_t size = 0;
};

struct FileControl
{
    size_t index = 0;                 // file index in files array in server memory
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
    using Callback = void (*)(const FileInfo&);
    FileInfo() = default;
    FileInfo(Attributes attributes, FileData data, Callback callback = nullptr) : attributes(attributes), data(data), callback(callback) {}
    Attributes attributes;
    FileData data;
    std::uint16_t amount_of_records = 0;
    bool is_open = false;
    Callback callback = nullptr; // callback on the end of write operation
};

struct RegisterInfo
{
    using Callback = void (*)(const RegisterInfo&);
    RegisterInfo() = default;
    RegisterInfo(Attributes attributes, std::uint16_t value, Callback callback = nullptr) : attributes(attributes), value(value), callback(callback) {}
    Attributes attributes;
    std::uint16_t value = 0;
    Callback callback = nullptr; // callback on the end of write operation
};

class BufferControl
{
public:
    void setSize(const std::uint8_t size) { buffer_size = size; }
    std::uint8_t getSize() const { return buffer_size; }

private:
    std::uint8_t buffer_size = 0;
};

class ServerResources
{
    using FileAccess = bool (*)(const std::uint8_t* data, const size_t size);

public:
    ServerResources(std::uint8_t record_size, FileAccess file_write, BufferControl* buffer_control)
        : record_size(record_size), fileWrite(file_write), buffer_control(buffer_control)
    {
    }
    bool writeRegister(const std::uint16_t address, const std::uint16_t value);
    bool readRegister(const std::uint16_t address, const std::uint16_t quantity, std::uint8_t* data, std::uint8_t& size);
    bool writeFile(const FileService& service, const std::uint8_t* data);
    bool readFile(const FileService& service, std::uint8_t* data, std::uint8_t& size);
    bool setupFile(const FileInfo& reg, const int index);
    bool setupRegister(const RegisterInfo& reg, const int index);
    static std::uint16_t extractHalfWord(const std::uint8_t* data);
    static void insertHalfWord(std::uint8_t* data, const std::uint16_t half_word);

private:
    const std::uint8_t record_size;
    BufferControl* buffer_control;
    FileAccess fileWrite;
    std::uint16_t file_record_counter;
    bool getAccessToRecord(const FileService& service, FileControl& control);
    bool fileOperationProcess(const int index);
    void resetFileOperation()
    {
        file_record_counter = 0;
        buffer_control->setSize(default_buffer_size);
    }
    std::array<RegisterInfo, RegisterDefinitions::getSize()> registers;
    std::array<FileInfo, FileDefinitions::getSize()> files;
};

} // namespace sm

#endif // SM_RESOURCES_HPP
