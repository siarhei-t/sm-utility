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

void DesktopTimer::platformStart() {}

void DesktopTimer::platformStop() {}

void DesktopCom::serverThread()
{
    std::vector<std::uint8_t> full_data;
    while (!thread_stop.load(std::memory_order_relaxed))
    {
        std::unique_lock<std::mutex> lk(m);
        blocker_reading.wait(lk, [this] { return buffer_support.ptr != nullptr; });
        if (thread_stop.load(std::memory_order_relaxed))
        {
            break;
        };
        full_data.clear();
        full_data.reserve(buffer_support.size);
        size_t total_bytes_read = 0;

        while (total_bytes_read < buffer_support.size)
        {
            size_t remaining = buffer_support.size - total_bytes_read;

            std::vector<std::uint8_t> partial_buf(remaining);
            size_t bytes_read = serial_port.readBinary(partial_buf, remaining);
            if (bytes_read == 0)
            {
                continue;
            }
            full_data.insert(full_data.end(), partial_buf.begin(), partial_buf.begin() + bytes_read);
            total_bytes_read += bytes_read;
        }
        if (buffer_support.size > 0)
        {
            std::copy(full_data.begin(), full_data.end(), buffer_support.ptr);
            reading_done = true;
            blocker_done.notify_one();
        }
    }
}

void DesktopCom::platformReadData(std::uint8_t data[], const size_t amount)
{
    buffer_support.ptr = data;
    buffer_support.size = amount;
    reading_done = false;
    // start reading in separated thread
    blocker_reading.notify_one();
    std::unique_lock<std::mutex> lk(m);
    blocker_done.wait(lk, [this] { return reading_done; });
}

void DesktopCom::platformSendData(std::uint8_t data[], const size_t amount)
{
    std::vector<std::uint8_t> tmp(data, data + amount);
    serial_port.writeBinary(tmp);
}

void DesktopCom::platformFlush() { serial_port.flushPort(); }

bool DesktopCom::platformInit()
{
    std::printf("platform init started...\n\n");
    std::string path = PlatformSupport::getPath();
    sp::PortConfig config = PlatformSupport::getConfig();
    auto error_code = serial_port.open(path);
    if (error_code)
    {
        std::printf("platform init error.\n");
        std::cout << "error: " << error_code.message() << "\n";
        return false;
    }
    error_code = serial_port.setup(config);
    if (error_code)
    {
        std::printf("platform init error.\n");
        std::cout << "error: " << error_code.message() << "\n";
        return false;
    }
    std::printf("platform configured.\n\n");
    return true;
}