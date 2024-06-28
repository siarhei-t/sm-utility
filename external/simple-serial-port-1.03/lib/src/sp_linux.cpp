/**
 * @file sp_linux.cpp
 *
 * @brief implementation for class defined in sp_linux.hpp
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/platform/sp_linux.hpp"
#include "../inc/sp_error.hpp"
#include <errno.h>

void SerialPortLinux::openPort(const std::string& path)
{
    std::string dev_path = "/dev/" + path;
    port_desc = open(dev_path.c_str(), O_RDWR);
    if (port_desc < 0)
    {
        throw std::system_error(sp::make_error_code(errno));
    }
}

void SerialPortLinux::closePort(void)
{
    close(port_desc);
    port_desc = -1;
}

void SerialPortLinux::flushPort()
{
    tcflush(port_desc, TCIOFLUSH);
}

void SerialPortLinux::setupPort(const sp::PortConfig& config)
{
    loadPortConfiguration();
    setDefaultPortConfiguration();
    setBaudRate(config.baudrate);
    setDataBits(config.data_bits);
    setParity(config.parity);
    setStopBits(config.stop_bits);
    setTimeOut(config.timeout_ms);
    savePortConfiguration();
}

void SerialPortLinux::writeString(const std::string& data)
{
    int stat = write(port_desc, data.c_str(), data.size());
    if (stat == -1)
    {
        throw std::system_error(sp::make_error_code(errno));
    }
}

void SerialPortLinux::writeBinary(const std::vector<std::uint8_t>& data)
{
    int stat = write(port_desc, data.data(), data.size());
    if (stat == -1)
    {
        throw std::system_error(sp::make_error_code(errno));
    }
}

size_t SerialPortLinux::readBinary(std::vector<std::uint8_t>& data,
                                   size_t length)
{
    ssize_t bytes_to_read = length;
    size_t bytes_read = 0;
    data.resize(length);
    while (bytes_to_read != 0)
    {
        ssize_t n = read(port_desc, data.data() + bytes_read, bytes_to_read);
        if (n < 0)
        {
            throw std::system_error(sp::make_error_code(errno));
        }
        else if ((n > 0) && (n <= bytes_to_read)) // reading
        {
            bytes_to_read = bytes_to_read - n;
            bytes_read = bytes_read + n;
        }
        else if (n == 0)
        {
            break;
        } // nothing to read
    }
    data.resize(bytes_read);
    tcflush(port_desc, TCIOFLUSH);
    return bytes_read;
}

void SerialPortLinux::setParity(const sp::PortParity parity)
{
    switch (parity)
    {
        case sp::PortParity::None:
            tty.c_cflag &= ~PARENB;
            break;

        case sp::PortParity::Even:
            tty.c_cflag |= PARENB;
            tty.c_cflag &= ~PARODD;
            break;

        case sp::PortParity::Odd:
            tty.c_cflag |= PARENB;
            tty.c_cflag |= PARODD;
            break;

        default:
            break;
    }
}

void SerialPortLinux::setBaudRate(const sp::PortBaudRate baudrate)
{
    switch (baudrate)
    {
        case sp::PortBaudRate::BD_9600:
            cfsetispeed(&(tty), B9600);
            cfsetospeed(&(tty), B9600);
            break;

        case sp::PortBaudRate::BD_19200:
            cfsetispeed(&(tty), B19200);
            cfsetospeed(&(tty), B19200);
            break;

        case sp::PortBaudRate::BD_38400:
            cfsetispeed(&(tty), B38400);
            cfsetospeed(&(tty), B38400);
            break;

        case sp::PortBaudRate::BD_57600:
            cfsetispeed(&(tty), B57600);
            cfsetospeed(&(tty), B57600);
            break;

        case sp::PortBaudRate::BD_115200:
            cfsetispeed(&(tty), B1152000);
            cfsetospeed(&(tty), B1152000);
            break;

        default:
            break;
    }
}

void SerialPortLinux::setDataBits(const sp::PortDataBits num_of_data_bits)
{
    tty.c_cflag &= ~CSIZE;

    switch (num_of_data_bits)
    {
        case sp::PortDataBits::Five:
            tty.c_cflag |= CS5;
            break;

        case sp::PortDataBits::Six:
            tty.c_cflag |= CS6;
            break;

        case sp::PortDataBits::Seven:
            tty.c_cflag |= CS7;
            break;

        case sp::PortDataBits::Eight:
            tty.c_cflag |= CS8;
            break;

        default:
            break;
    }
}

void SerialPortLinux::setStopBits(const sp::PortStopBits num_of_stop_bits)
{
    switch (num_of_stop_bits)
    {
        case sp::PortStopBits::One:
            tty.c_cflag &= ~CSTOPB;
            break;

        case sp::PortStopBits::Two:
            tty.c_cflag |= CSTOPB;
            break;

        default:
            break;
    }
}

void SerialPortLinux::setTimeOut(const int timeout_ms)
{
    const unsigned char max_timeout = 0xFF;
    tty.c_cc[VMIN] = 0;
    int timeout = timeout_ms / 100;
    if (timeout > max_timeout)
    {
        throw std::system_error(sp::make_error_code(errno));
    }
    else
    {
        tty.c_cc[VTIME] = timeout;
    }
}

void SerialPortLinux::loadPortConfiguration()
{
    int stat = tcgetattr(port_desc, &(tty));
    if (stat != 0)
    {
        throw std::system_error(sp::make_error_code(errno));
    }
}

void SerialPortLinux::savePortConfiguration()
{
    int stat = tcsetattr(port_desc, TCSANOW, &(tty));
    if (stat != 0)
    {
        throw std::system_error(sp::make_error_code(errno));
    }
}

void SerialPortLinux::setDefaultPortConfiguration()
{
    // hardware flow control disabled
    tty.c_cflag &= ~CRTSCTS;
    // disable software flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    // only raw data
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    // read enabled and ctrl lines ignored
    tty.c_cflag |= CREAD | CLOCAL;
    // all features disabled (echo, new lines, modem modes, etc.)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~ONLCR;
}
