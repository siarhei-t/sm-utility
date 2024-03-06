/**
 * @file sp_types.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#ifndef SP_TYPES_H
#define SP_TYPES_H

//debug purpose, select one 
//#define PLATFORM_WINDOWS 1
//#define PLATFORM_LINUX   1

namespace sp
{
    enum class PortState
    {
        Open,
        Close
    };

    enum class PortBaudRate
    {
        BD_9600,
        BD_19200,
        BD_38400,
        BD_57600,
        BD_115200
    };

    enum class PortDataBits
    {
        Five,
        Six,
        Seven,
        Eight
    };

    enum class PortParity 
    {
        None,
        Even,
        Odd
    };

    enum class PortStopBits
    {
        One,
        Two
    };

    struct PortConfig
    {
        PortBaudRate baudrate   = PortBaudRate::BD_9600;
        PortDataBits data_bits  = PortDataBits::Eight;
        PortParity   parity     = PortParity::None;
        PortStopBits stop_bits  = PortStopBits::One;
        int          timeout_ms = 1000;
    };
}

#endif //SP_TYPES_H
