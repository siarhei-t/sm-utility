/**
 * @file serial_port.cpp
 *
 * @brief implementation for class defined in serial_port.hpp
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/serial_port.hpp"
#if defined(PLATFORM_LINUX)
#include <dirent.h>
#include <sys/types.h>
#elif defined(PLATFORM_WINDOWS)
#include <windows.h>
#else
#error "target platform not defined."
#endif

using namespace sp;

SerialPort::SerialPort(std::string name) { (void)open(name); }

SerialPort::SerialPort(std::string name, sp::PortConfig config)
{
    (void)open(name);
    (void)setup(config);
}

std::error_code SerialPort::open(const std::string name)
{
    std::error_code error(0, sp::sp_category());
    try
    {
        port.openPort(name);
        state = sp::PortState::Open;
        path = name;
    }
    catch (const std::system_error& e)
    {
        error = e.code();
    }
    return error;
}

void SerialPort::close()
{
    port.closePort();
    state = sp::PortState::Open;
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
        std::string device = "COM" + std::to_string(i);
#if defined(UNICODE)
        std::wstring converted_device =
            std::wstring(device.begin(), device.end());
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
