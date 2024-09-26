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
#include "../../../core/external/simple-serial-port/inc/serial_port.hpp"

class PlatformSupport
{
public:
    void setPath(std::string& new_path)
    {
        path = new_path;
    }
    void setConfig(sp::PortConfig& config)
    {
        this->config = config;
    }
    static std::string& getPath()
    {
        return path;
    }
    static sp::PortConfig& getConfig()
    {
        return config;
    }
private:
    static std::string path;
    static sp::PortConfig config;
};

class DesktopCom : public sm::Com<DesktopCom>
{
public:
    bool platformInit();
    void platformSendData(std::uint8_t data[], const size_t amount);
    void platformReadData(std::uint8_t data[], const size_t amount);
    void platformFlush();

private:
    sp::SerialPort serial_port;
};

class DesktopTimer : public sm::Timer<DesktopTimer>
{
public:
 void platformStart();
 void platformStop();
};

#endif // PLATFORM_HPP
