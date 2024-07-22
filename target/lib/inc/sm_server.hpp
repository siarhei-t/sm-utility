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

#include <array>
#include <cstddef>
#include "sm_modbus.hpp"

namespace sm 
{

enum class ServerExceptions
{
    no_error,
    address_not_recognized,
    bad_crc,
    function_exception
};

enum class ServerRegisters
{
    file_control = 0, // file control register address, access W
    update_request,   //firmware update request register, access W
    app_erase,        // firmware erase request register, access W
    app_start,        // application start register, access W
    boot_status,      // bootloader status register, access R
    record_size,      // common record size in files on server, access R
    count            // enum element counter
};

enum class ServerFiles
{
    application = 1, // id for file with server firmware, access W
    metadata,        // id for file with server metadata, access R
    count            // enum element counter
};

struct FileService
{
    FileService() = default;
    FileService(std::uint16_t file_id, std::uint16_t record_id, std::uint16_t length):
    file_id(file_id), record_id(record_id), length(length){}
    
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
    bool property_encrypted = false;  //is file encrypted
    bool property_read = false;       // is file readable
    bool property_write = false;      // is file writable
    std::uint16_t num_of_records = 0; // number of records in file
    std::uint32_t file_size = 0;      // file size in bytes
    std::uint8_t*  p_data = nullptr;  // pointer to file data

};

class ServerResources
{
    public:
    /**
     * @brief Construct a new Server Resources object
     * 
     */
    ServerResources() = default;
    /**
     * @brief 
     * 
     * @param address 
     * @param value 
     * @return true 
     * @return false 
     */
    bool writeRegister(const std::uint16_t address, const std::uint16_t value) const;
    /**
     * @brief 
     * 
     * @param address 
     * @param quantity 
     * @return true 
     * @return false 
     */
    bool readRegister(const std::uint16_t address, const std::uint16_t quantity) const;
    /**
     * @brief 
     * 
     * @param service 
     * @param data 
     * @return true 
     * @return false 
     */
    bool writeFile(const FileService& service, const std::uint8_t data[]) const;
    /**
     * @brief 
     * 
     * @param service 
     * @return true 
     * @return false 
     */
    bool readFile(const FileService& service) const;
    
    private:

    std::array<std::uint16_t, static_cast<std::size_t>(ServerRegisters::count)> registers;
    std::array<FileInfo, static_cast<std::size_t>(ServerFiles::count)>files;

};

class ModbusServer
{
    public:
    /**
     * @brief Construct a new Modbus Server object
     * 
     * @param address server address
     */
    ModbusServer(std::uint8_t address) : address(address){}
    /**
     * @brief 
     * 
     * @param data 
     * @return ServerExceptions 
     */
    ServerExceptions serverTask(std::uint8_t data[]) const;

    private:
        // server address
        std::uint8_t address = 0;
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
        * @return modbus::Exceptions 
        */
        modbus::Exceptions readRegister(std::uint8_t data[]) const;
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
        * @return modbus::Exceptions 
        */
        modbus::Exceptions readFile(std::uint8_t data[]) const;
        /**
         * @brief 
         * 
         * @param data 
         * @return std::uint16_t 
         */
        std::uint16_t extractHalfWord(const std::uint8_t data[]) const;

};

} // namespace sm

#endif // SM_SERVER_HPP
