/**
 * @file sp_windows.cpp
 *
 * @brief implementation for class defined in sp_windows.hpp
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/platform/sp_windows.hpp"
#include "../inc/sp_error.hpp"

void SerialPortWindows::openPort(const std::string& path)
{
#if defined(UNICODE)
    std::wstring converted_path = std::wstring(path.begin(), path.end());
    port_desc = CreateFile(converted_path.c_str(), GENERIC_READ | GENERIC_WRITE,
                           0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
#else
    port_desc = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
#endif
    if (port_desc == INVALID_HANDLE_VALUE)
    {
        throw std::system_error(sp::make_error_code(GetLastError()));
    }
}

void SerialPortWindows::closePort(void)
{
    CloseHandle(port_desc);
    port_desc = INVALID_HANDLE_VALUE;
}

void SerialPortWindows::setupPort(const sp::PortConfig& config)
{
    loadPortConfiguration();
    setBaudRate(config.baudrate);
    setDataBits(config.data_bits);
    setParity(config.parity);
    setStopBits(config.stop_bits);
    savePortConfiguration();
    setTimeOut(config.timeout_ms);
}

void SerialPortWindows::writeString(const std::string& data)
{
    DWORD bytes_written;
    WINBOOL stat =
        WriteFile(port_desc, data.c_str(), data.size(), &bytes_written, NULL);
    if (stat == 0)
    {
        throw std::system_error(sp::make_error_code(GetLastError()));
    }
}

void SerialPortWindows::writeBinary(const std::vector<std::uint8_t>& data)
{
    DWORD bytes_written;
    WINBOOL stat =
        WriteFile(port_desc, data.data(), data.size(), &bytes_written, NULL);
    if (stat == 0)
    {
        throw std::system_error(sp::make_error_code(GetLastError()));
    }
}

size_t SerialPortWindows::readBinary(std::vector<std::uint8_t>& data,
                                     size_t length)
{
    size_t bytes_to_read = length;
    DWORD bytes_read = 0;
    data.resize(length);
    while (bytes_to_read != 0)
    {
        WINBOOL n = ReadFile(port_desc, data.data() + bytes_read, length,
                             &bytes_read, NULL);
        if (n == 0)
        {
            throw std::system_error(sp::make_error_code(GetLastError()));
        }
        else if ((bytes_read > 0) && (bytes_read <= bytes_to_read))
        {
            bytes_to_read = bytes_to_read - bytes_read;
        } // reading
        else if (bytes_read == 0)
        {
            break;
        } // nothing to read
    }
    data.resize(bytes_read);
    FlushFileBuffers(port_desc);
    return bytes_read;
}

void SerialPortWindows::setParity(const sp::PortParity parity)
{
    switch (parity)
    {
        case sp::PortParity::None:
            tty.Parity = NOPARITY;
            break;

        case sp::PortParity::Even:
            tty.Parity = EVENPARITY;
            break;

        case sp::PortParity::Odd:
            tty.Parity = ODDPARITY;
            break;

        default:
            break;
    }
}

void SerialPortWindows::setBaudRate(const sp::PortBaudRate baudrate)
{
    switch (baudrate)
    {
        case sp::PortBaudRate::BD_9600:
            tty.BaudRate = CBR_9600;
            break;

        case sp::PortBaudRate::BD_19200:
            tty.BaudRate = CBR_19200;
            break;

        case sp::PortBaudRate::BD_38400:
            tty.BaudRate = CBR_38400;
            break;

        case sp::PortBaudRate::BD_57600:
            tty.BaudRate = CBR_57600;
            break;

        case sp::PortBaudRate::BD_115200:
            tty.BaudRate = CBR_115200;
            break;

        default:
            break;
    }
}

void SerialPortWindows::setDataBits(const sp::PortDataBits num_of_data_bits)
{
    switch (num_of_data_bits)
    {
        case sp::PortDataBits::Five:
            tty.ByteSize = 5;
            break;

        case sp::PortDataBits::Six:
            tty.ByteSize = 6;
            break;

        case sp::PortDataBits::Seven:
            tty.ByteSize = 7;
            break;

        case sp::PortDataBits::Eight:
            tty.ByteSize = 8;
            break;

        default:
            break;
    }
}

void SerialPortWindows::setStopBits(const sp::PortStopBits num_of_stop_bits)
{
    switch (num_of_stop_bits)
    {
        case sp::PortStopBits::One:
            tty.StopBits = ONESTOPBIT;
            break;

        case sp::PortStopBits::Two:
            tty.StopBits = TWOSTOPBITS;
            break;

        default:
            break;
    }
}

void SerialPortWindows::setTimeOut(const int timeout_ms)
{
    COMMTIMEOUTS timeouts;
    WINBOOL stat = GetCommTimeouts(port_desc, &timeouts);
    if (stat == 0)
    {
        throw std::system_error(sp::make_error_code(GetLastError()));
    }
    timeouts.ReadTotalTimeoutConstant = timeout_ms;
    timeouts.WriteTotalTimeoutConstant = timeout_ms;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadIntervalTimeout = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    stat = SetCommTimeouts(port_desc, &timeouts);
    if (stat == 0)
    {
        throw std::system_error(sp::make_error_code(GetLastError()));
    }
}

void SerialPortWindows::loadPortConfiguration()
{
    WINBOOL stat = GetCommState(port_desc, &tty);
    if (stat == 0)
    {
        throw std::system_error(sp::make_error_code(GetLastError()));
    }
}

void SerialPortWindows::savePortConfiguration()
{
    WINBOOL stat = SetCommState(port_desc, &tty);
    if (stat == 0)
    {
        throw std::system_error(sp::make_error_code(GetLastError()));
    }
}
