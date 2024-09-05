/**
 * @file platform.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include "../../../core/server/inc/sm_com.hpp"
#include "../../../core/external/simple-serial-port-1.03/lib/inc/serial_port.hpp"

class DesktopCom : public sm::Com
{
public:

private:
    sp::SerialPort serial_port;
    //void platformFlush();
    void platformReadData(std::uint8_t data[], const size_t amount) override;
    //void platformSendData(std::uint8_t data[], const size_t amount);
};

class DesktopTimer : public sm::Timer
{

};

#endif // PLATFORM_HPP
