/**
 * @file platform.cpp
 *
 * @brief 
 *
 * @author
 *
 */

#include "platform.hpp"
#include <cstdio>
#include <iostream>
#include <vector>

std::string PlatformSupport::path;
sp::PortConfig PlatformSupport::config;

void DesktopTimer::platformStart()
{

}

void DesktopTimer::platformStop()
{

}

void DesktopCom::serverThread()
{
    std::vector<std::uint8_t> data;
    
    while (!thread_stop.load(std::memory_order_relaxed))
    {
        std::unique_lock<std::mutex>lk(m);
        blocker.wait(lk);
        size_t bytes_read = 0;
        while(bytes_read != buffer_support.buffer_size)
        {
            bytes_read = serial_port.readBinary(data, buffer_support.buffer_size);
            if(bytes_read != buffer_support.buffer_size)
            {
                std::printf("port reading timeout !\n");
            }
        }
        std::copy(data.begin(), data.end(), buffer_support.buffer_ptr);
        data_ready.store(true, std::memory_order_relaxed);
        std::printf("port reading finished.\n");
    }
}

void DesktopCom::platformReadData(std::uint8_t data[], const size_t amount)
{
    buffer_support.buffer_ptr = data;
    buffer_support.buffer_size = amount;
    // start reading in separated thread
    blocker.notify_one();
}

void DesktopCom::platformSendData(std::uint8_t data[], const size_t amount)
{
    std::vector<std::uint8_t> tmp(data,data + amount);
    serial_port.writeBinary(tmp);
}

void DesktopCom::platformFlush()
{
    serial_port.flushPort();
}

bool DesktopCom::platformInit()
{
    std::printf("platform init started...\n\n");
    std::string path = PlatformSupport::getPath();
    sp::PortConfig config = PlatformSupport::getConfig();
    auto error_code = serial_port.open(path);
    if(error_code)
    {
        std::printf("platform init error.\n");
        std::cout<<"error: "<<error_code.message()<<"\n";
        return false;
    }
    error_code = serial_port.setup(config);
    if(error_code)
    {
        std::printf("platform init error.\n");
        std::cout<<"error: "<<error_code.message()<<"\n";
        return false;
    }
    std::printf("platform configured.\n\n");
    return true;
}