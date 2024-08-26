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
#include "../../common/sm_common.hpp"
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
    std::uint8_t index = 0;           // file index in files array in chip memory
    std::uint8_t* p_record = nullptr; // pointer to record
    std::uint8_t length = 0;          // actual record length in bytes
};

struct FileInfo
{
    bool property_encrypted = false;  // is file encrypted
    bool property_read = false;       // is file readable
    bool property_write = false;      // is file writable
    std::uint16_t num_of_records = 0; // number of records in file
    std::uint32_t file_size = 0;      // file size in bytes
    std::uint8_t* p_data = nullptr;   // pointer to file data
};

struct RegisterInfo
{
    bool property_write;
    bool property_read;
    std::uint16_t value;
};

template<size_t amount_of_regs, size_t amount_of_files> class ServerResources
{

public:

    bool writeRegister(const std::uint16_t address, const std::uint16_t value) const;

    bool readRegister(ServerCallback& server_callback, const std::uint16_t address, const std::uint16_t quantity) const;

    bool writeFile(const FileService& service, const std::uint8_t data[]) const;

    bool readFile(ServerCallback& server_callback, const FileService& service) const;

private:
    std::array<RegisterInfo, amount_of_regs> registers;
    std::array<FileInfo, amount_of_files> files;
};

class ModbusServer
{
public:

    ModbusServer(std::uint8_t address) : address(address) {}

    ServerExceptions serverTask(std::uint8_t data[]);

    std::uint8_t getDataUnitSize() const { return pdu_size; }

    static std::uint16_t extractHalfWord(const std::uint8_t data[]);

    static void insertHalfWord(std::uint8_t data[], const std::uint16_t half_word);

private:
    // server address
    std::uint8_t address = 0;
    // ServerResources instance with access to registers and files
    //ServerResources server_resources;
    // actual pdu size
    std::uint8_t pdu_size = 0;
    /**
     * @brief server method for modbus::write_reg function
     *
     * @param data pointer to array with received message
     * @return modbus::Exceptions
     */
    modbus::Exceptions writeRegister(std::uint8_t data[]) const;
    /**
     * @brief server method for modbus::read_regs function
     *
     * @param data pointer to array with received message
     * @param server_callback reference to expected answer from server
     * @return modbus::Exceptions
     */
    modbus::Exceptions readRegister(std::uint8_t data[], ServerCallback& server_callback) const;
    /**
     * @brief
     *
     * @param data
     * @return modbus::Exceptions
     */
    modbus::Exceptions writeFile(std::uint8_t data[]) const;
    /**
     * @brief
     *
     * @param data
     * @param server_callback
     * @return modbus::Exceptions
     */
    modbus::Exceptions readFile(std::uint8_t data[], ServerCallback& server_callback) const;
};

} // namespace sm

#endif // SM_SERVER_HPP
