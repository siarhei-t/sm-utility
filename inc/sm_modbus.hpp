/**
 * @file sm_modbus.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#ifndef SM_MODBUS_H
#define SM_MODBUS_H

#include <cstdint>
#include <vector>

namespace modbus
{
    enum class FunctionCodes
    {
        read_registers = 0x03, // read holding registers
        write_register = 0x06, // write single register
        read_file      = 0x14, // read file records
        write_file     = 0x15, // write file records 
        undefined      = 0xFF, // illegal function code
    };
    
    enum class ModbusMode
    {
        rtu,
        ascii
    };

    class ModbusClient
    {
        public:
        ModbusClient() = default;
        ModbusMode getMode() const {return mode;};
        void setMode(const ModbusMode new_mode) {mode = new_mode;};
        /// @brief write record to file according to Modbus protocol
        /// @param addr server address
        /// @param file_id file id
        /// @param record_id record id
        /// @param record_data vector with record data
        /// @return reference to vector with created message
        std::vector<std::uint8_t>& msgWriteFileRecord(const std::uint8_t addr, 
                                                      const std::uint16_t file_id,
                                                      const std::uint16_t record_id,
                                                      const std::vector<std::uint8_t>& record_data);
        /// @brief read file record according to Modbus protocol
        /// @param addr server address
        /// @param file_id file id
        /// @param record_id record id
        /// @param length record length
        /// @return reference to vector with created message
        std::vector<std::uint8_t>& msgReadFileRecord(const std::uint8_t addr,
                                                     const std::uint16_t file_id,
                                                     const std::uint16_t record_id,
                                                     const std::uint16_t length);
        /// @brief write date to one register according to Modbus
        /// @param addr server address
        /// @param reg start address
        /// @param value half word to write
        /// @return reference to vector with created message
        std::vector<std::uint8_t>& msgWriteRegister(const std::uint8_t addr, const std::uint16_t reg, const std::uint16_t value);
        /// @brief read holding registers according to Modbus protocol
        /// @param addr server address
        /// @param reg register start address
        /// @param quantity amount of registers to read
        /// @return reference to vector with created message
        std::vector<std::uint8_t>& msgReadRegisters(const std::uint8_t addr, const std::uint16_t reg,const std::uint16_t quantity);

        private:
            ModbusMode mode = ModbusMode::rtu;
             /// @brief internal message buffer, used to store last created message
             std::vector<std::uint8_t> buffer;
            /// @brief generate ADU on internal buffer
            /// @param addr server address
            /// @param func function code
            /// @param data vector with data for PDU
            void createMessage(const std::uint8_t addr, const std::uint8_t func, const std::vector<std::uint8_t>& data);
            /// @brief calculate crc16
            /// @param data vector with data to calculate crc
            /// @return calculated crc
            std::uint16_t crc16(const std::vector<std::uint8_t>& data);
    };
}

#endif //SM_MODBUS_H
