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

#include "sm_modbus.hpp"

namespace sm 
{

enum class ServerRegisters
{
    file_control = 0,   // file control register address, access W
    update_request = 1, // firmware update request register, access W
    app_erase = 2,      // firmware erase request register, access W
    app_start = 3,      // application start register, access W
    boot_status = 5,    // bootloader status register, access R
    record_size = 6,    // common record size in files on server, access R
};

enum class ServerFiles
{
    application = 1, // id for file with server firmware, access W
    metadata = 2     // id for file with server metadata, access R
};

struct FileService
{
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
    bool property_encrypted = false; //is file encrypted
    bool property_read = false;      // is file readable
    bool property_write = false;     // is file writable
    std::uint16_t num_of_records;    // number of records in file
    std::uint32_t file_size;         // file size in bytes
    std::uint8_t*  p_data;           // pointer to file data

};

class ModbusServer
{
    public:
    /**
     * @brief 
     * 
     * @param data 
     * @return modbus::Exceptions 
     */
    modbus::Exceptions writeRegister(std::uint8_t data[]) const;
    /**
     * @brief 
     * 
     * @param data 
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

    private:
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
        bool writeFile(const FileService& service, const uint8_t data[]) const;
        /**
         * @brief 
         * 
         * @param service 
         * @return true 
         * @return false 
         */
        bool readFile(const FileService& service) const;

};

} // namespace sm

#endif // SM_SERVER_HPP
