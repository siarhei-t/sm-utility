/**
 * @file main.cpp
 *
 * @brief
 *
 * @author
 *
 */

#include "../../../core/common/sm_log.hpp"
#include "../../../core/server/inc/sm_node.hpp"
#include "platform.hpp"
#include <cassert>

#define ENABLE_LOG_INFO
// #define ENABLE_LOG_DEBUG

#ifdef ENABLE_LOG_INFO
#undef LOG_INFO
#define LOG_INFO std::printf
#endif

#ifdef ENABLE_LOG_DEBUG
#undef LOG_DEBUG
#define LOG_DEBUG std::printf
#endif

constexpr std::uint8_t record_size = 208;

PlatformSupport platform_support;

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        LOG_INFO("incorrect arguments list passed, exit...\n");
        return 0;
    }
    std::string path_to_port = argv[1];
    std::string address_str = argv[2];
    std::uint8_t address;
    try
    {
        auto number = std::stoi(address_str);
        if ((number > modbus::max_rtu_address) || (number < modbus::min_rtu_address))
        {
            LOG_INFO("out of range address passed, exit...\n");
            return 0;
        }
        else
        {
            address = static_cast<std::uint8_t>(number);
        }
    }
    catch (std::invalid_argument const& ex)
    {
        LOG_INFO("invalid argument passed, exit...\n");
        return 0;
    }

    // FIXME: fix it to read config from command line int the future
    sp::PortConfig config;
    config.baudrate = sp::PortBaudRate::BD_57600;
    config.timeout_ms = 2000;

    platform_support.setPath(path_to_port);
    platform_support.setConfig(config);

    sm::DataNode<DesktopCom, DesktopTimer, DesktopWaitPolicy> data_node(address, record_size);

    data_node.start();
    data_node.loop();
}
