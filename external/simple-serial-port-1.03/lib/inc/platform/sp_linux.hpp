/**
 * @file sp_linux.hpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */
#ifndef SP_LINUX_H
#define SP_LINUX_H

#include "../sp_types.hpp"
#include <cstdint>
#include <errno.h>
#include <fcntl.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

class SerialPortLinux
{
public:
    SerialPortLinux() = default;
    ~SerialPortLinux() { closePort(); }
    /// @brief open port
    /// @param path string with path to device
    void openPort(const std::string& path);
    /// @brief close actual port if opened
    void closePort();
    /// @brief setup port with new configuration
    /// @param config struct with port configuration
    void setupPort(const sp::PortConfig& config);
    /// @brief write string data to actual port
    /// @param data string object with data to send
    void writeString(const std::string& data);
    /// @brief write raw data to actual port
    /// @param data string object with data to send
    void writeBinary(const std::vector<std::uint8_t>& data);
    /// @brief read raw data from port
    /// @param data reference to vector with buffer for data
    /// @param length how many bytes we expect to read during timeout
    /// @returns how many bytes we read actually
    size_t readBinary(std::vector<std::uint8_t>& data, size_t length);
    /// @brief reset internal OS buffers
    void flushPort();

private:
    /// @brief opened port file descriptor
    int port_desc = -1;
    /// @brief struct with port configuration
    struct termios tty = {};
    /// @brief setup parity bits in tty struct
    /// @param parity expected parity mode
    void setParity(const sp::PortParity parity);
    // \brief setup baudrate in tty struct
    // \param baudrate expected port baudrate
    void setBaudRate(const sp::PortBaudRate baudrate);
    // \brief setup data bits in tty struct
    // \param num_of_data_bits expected number of data bits in frame
    void setDataBits(const sp::PortDataBits num_of_data_bits);
    // \brief setup stop bits in tty struct
    // \param num_of_stop_bits expected number of stop bits in frame
    void setStopBits(const sp::PortStopBits num_of_stop_bits);
    // \brief setup timeout in tty struct
    // \param timeout_ms expected timeout i ms for data read
    void setTimeOut(const int timeout_ms);
    // \brief load actual configuration to the tty struct
    void loadPortConfiguration();
    // \brief save tty struct as actual configuration
    void savePortConfiguration();
    // \brief load default configuration to the tty struct
    void setDefaultPortConfiguration();
};

#endif // SP_LINUX_H
