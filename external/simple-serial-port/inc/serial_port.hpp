/**
 * @file serial_port.hpp
 *
 * @brief serial port interface class definition
 *
 * @author Siarhei Tatarchanka
 *
 */

#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <cstdint>
#include <string>
#include <vector>

#include "../inc/sp_error.hpp"
#include "../inc/sp_types.hpp"
#if defined(PLATFORM_LINUX)
#include "../inc/platform/sp_linux.hpp"
#elif defined(PLATFORM_WINDOWS)
#include "../inc/platform/sp_windows.hpp"
#else
#error "target platform not defined."
#endif

namespace sp
{
class SerialPort
{
public:
    /// @brief default coustructor
    SerialPort() = default;
    /// @brief constructor that will open port
    /// @param name port name to open
    SerialPort(std::string name);
    /// @brief constructor that will open port and configure it
    /// @param name port name to open
    /// @param config port configuration
    SerialPort(std::string name, sp::PortConfig config);
    /// @brief open port with passed name
    /// @param name port name
    /// @return error code
    std::error_code open(const std::string name);
    /// @brief close port
    void close();
    /// @brief setup port with passed configuration (if port is open)
    /// @param config port configuration
    /// @return error enum
    std::error_code setup(sp::PortConfig config);
    /// @brief request for port system path
    /// @return actual path
    std::string getPath() const { return path; };
    /// @brief request for port configuration
    /// @return struct with configuration
    sp::PortConfig getConfig() const { return config; };
    /// @brief request for port state
    /// @return actual port state
    sp::PortState getState() const { return state; };
#if defined(PLATFORM_LINUX)
    SerialPortLinux port;
#elif defined(PLATFORM_WINDOWS)
    SerialPortWindows port;
#endif

private:
    /// @brief actual port state
    sp::PortState state = sp::PortState::Close;
    /// @brief actual port path
    std::string path = "dev/null";
    /// @brief actual port config
    sp::PortConfig config = sp::PortConfig();
};

class SerialDevice
{
public:
    SerialDevice() { updateAvailableDevices(); };
    /// @brief plaform depended call to update list with serial port devices
    void updateAvailableDevices();
    /// @brief request for list with serial port devices in system
    /// @return referense to a vector with devices
    std::vector<std::string>& getListOfAvailableDevices() { return devices; };
    /// @brief request for list with serial port devices in system
    /// @return vector with devices
    std::vector<std::string> getListOfAvailableDevices() const
    {
        return devices;
    };

private:
    /// @brief vertor with actual list of available serial ports
    std::vector<std::string> devices;
};
} // namespace sp

#endif // SERIAL_PORT_H
