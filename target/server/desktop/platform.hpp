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

#include "../../../core/external/simple-serial-port/inc/serial_port.hpp"
#include "../../../core/server/inc/com.hpp"
#include "../../../core/server/inc/timer.hpp"
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>

struct BufferSupport
{
    size_t size = 0;
    std::uint8_t* ptr = nullptr;
};

class PlatformSupport
{
public:
    void setPath(std::string& new_path) { path = new_path; }
    void setConfig(sp::PortConfig& config) { this->config = config; }
    static std::string& getPath() { return path; }
    static sp::PortConfig& getConfig() { return config; }

private:
    static std::string path;
    static sp::PortConfig config;
};

struct DesktopWaitPolicy
{
    static void wait() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
};

class DesktopTimer : public sm::Timer<DesktopTimer>
{
public:
    void platformStart();
    void platformStop();
};

class DesktopCom : public sm::Com<DesktopCom>
{
public:
    DesktopCom() : server_thread(&DesktopCom::serverThread, this) {}
    ~DesktopCom()
    {
        thread_stop.store(true, std::memory_order_relaxed);
        blocker_reading.notify_one();
        server_thread.join();
    }
    bool platformInit();
    void platformSendData(std::uint8_t data[], const size_t amount);
    void platformReadData(std::uint8_t data[], const size_t amount);
    void platformFlush();

private:
    std::mutex m;
    bool reading_done = false;
    std::condition_variable blocker_reading;
    std::condition_variable blocker_done;
    std::thread server_thread;
    std::atomic<bool> thread_stop{false};
    sp::SerialPort serial_port;
    BufferSupport buffer_support;
    void serverThread();
};

#endif // PLATFORM_HPP
