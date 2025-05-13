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

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include "../../../core/server/inc/sm_com.hpp"
#include "../../../core/server/inc/sm_timer.hpp"
#include "../../../core/server/inc/sm_node.hpp"
#include "../../../core/external/simple-serial-port/inc/serial_port.hpp"

struct BufferSupport
{
    size_t buffer_size = 0;
    std::uint8_t* buffer_ptr = nullptr;
};

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

struct DesktopWaitPolicy
{
    static void wait()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
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
        blocker.notify_one();
        thread_stop.store(true, std::memory_order_relaxed);
        server_thread.join();
    }
    bool platformInit();
    void platformSendData(std::uint8_t data[], const size_t amount);
    void platformReadData(std::uint8_t data[], const size_t amount);
    void platformFlush();

private:
    std::mutex m;
    std::condition_variable blocker;
    std::thread server_thread;
    std::atomic<bool> thread_stop{false};
    sp::SerialPort serial_port;
    BufferSupport buffer_support;
    void serverThread();
};

#endif // PLATFORM_HPP
