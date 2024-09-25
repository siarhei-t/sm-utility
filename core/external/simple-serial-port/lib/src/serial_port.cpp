/**
 * @file serial_port.cpp
 *
 * @brief implementation for class defined in serial_port.hpp
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/serial_port.hpp"
#include "../inc/sp_error.hpp"
#if defined(PLATFORM_LINUX)
#include <dirent.h>
#include <sys/types.h>
#elif defined(PLATFORM_WINDOWS)
#include <windows.h>
#else
#error "target platform not defined."
#endif

using namespace sp;

SerialPort::SerialPort(std::string name)
{
    std::error_code error_code = open(name);
    if(error_code)
    {
        throw std::system_error(error_code);
    }
}

SerialPort::SerialPort(std::string name, sp::PortConfig config)
{
    std::error_code error_code = open(name);
    if(error_code)
    {
        throw std::system_error(error_code);
    }
    error_code = setup(config);
    if(error_code)
    {
        throw std::system_error(error_code);
    }
}

std::error_code SerialPort::open(const std::string name)
{
    std::error_code error_code = std::error_code();
    try
    {
        port.openPort(name);
        port.flushPort();
        state = sp::PortState::Open;
        path = name;
    }
    catch (const std::system_error& e)
    {
        error_code = e.code();
    }
    return error_code;
}

void SerialPort::close()
{
    port.closePort();
    state = sp::PortState::Close;
    path = "dev/null";
}

std::error_code SerialPort::setup(sp::PortConfig config)
{
    std::error_code error(0, sp::sp_category());
    try
    {
        port.setupPort(config);
        this->config = config;
    }
    catch (const std::system_error& e)
    {
        error = e.code();
    }
    return error;
}

void SerialDevice::updateAvailableDevices()
{
    devices.clear();

#if defined(PLATFORM_WINDOWS)

#if defined(UNICODE)
    wchar_t dev_path[256];
#else
    char dev_path[256];
#endif
    for (auto i = 0; i < 255; ++i)
    {
        std::string templates[] = {"COM" + std::to_string(i),
                                   "CNCA" + std::to_string(i),
                                   "CNCB" + std::to_string(i)};

        for(std::string &device : templates)
        {
            #if defined(UNICODE)
            std::wstring converted_device = std::wstring(device.begin(), device.end());
            DWORD result = QueryDosDevice(converted_device.c_str(), dev_path, 256);
            #else
            DWORD result = QueryDosDevice(device.c_str(), dev_path, 256);
            #endif
            if (result != 0)
            {
                devices.push_back(device);
            }
            else
            {
                continue;
            }
        }
    }
#elif defined(PLATFORM_LINUX)
    const char path[] = {"/dev/"};
    const std::string dev_template[] = {"ttyUSB", "ttyACM"};
    dirent* dp;
    DIR* dirp;
    dirp = opendir(path);
    while ((dp = readdir(dirp)) != NULL)
    {
        size_t tab_size = sizeof(dev_template) / sizeof(std::string);
        std::string device = dp->d_name;

        for (size_t i = 0; i < tab_size; ++i)
        {
            int result =
                device.compare(0, dev_template[i].size(), dev_template[i]);
            if (result == 0)
            {
                devices.push_back(device);
            }
            else
            {
                continue;
            }
        }
    }
    (void)closedir(dirp);
#endif
}
