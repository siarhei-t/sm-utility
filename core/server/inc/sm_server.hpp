/**
 * @file sm_server.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_SERVER_HPP
#define SM_SERVER_HPP

#include "../../common/sm_modbus.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace sm
{

enum class ServerExceptions
{
    no_error,
    address_not_recognized,
    bad_crc,
    function_exception
};

struct ServerCallback
{
    std::uint8_t* data = nullptr; // pointer to pdu data start expected
    std::uint8_t length = 0;      // pdu data length
};

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
    size_t index = 0;                // file index in files array in chip memory
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

    bool readRegister(const std::uint16_t address, const std::uint16_t quantity, std::uint8_t& length);

    bool writeFile(const FileService& service, const std::uint8_t data[]);

    bool readFile(const FileService& service, std::uint8_t& length);

private:
    std::array<RegisterInfo, amount_of_regs> registers;
    std::array<FileInfo, amount_of_files> files;
};

template<size_t amount_of_regs, size_t amount_of_files, std::uint8_t record_define> class ModbusServer
{
public:
    ModbusServer(std::uint8_t address) : address(address), record_size(record_define) {}

    ServerExceptions serverTask(std::uint8_t data[], const std::uint8_t length);

private:
    const std::uint8_t address;
    const std::uint8_t record_size;

    ServerResources<amount_of_regs, amount_of_files> server_resources;

    modbus::Exceptions writeRegister(std::uint8_t data[]);

    modbus::Exceptions readRegister(std::uint8_t data[], std::uint8_t& length);

    modbus::Exceptions writeFile(std::uint8_t data[]);

    modbus::Exceptions readFile(std::uint8_t data[], std::uint8_t& length);
    
    std::uint16_t extractHalfWord(const std::uint8_t data[]);
    
    void insertHalfWord(std::uint8_t data[], const std::uint16_t half_word);

};

} // namespace sm

#endif // SM_SERVER_HPP
