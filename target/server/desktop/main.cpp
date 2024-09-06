/**
 * @file main.cpp
 *
 * @brief 
 *
 * @author
 *
 */

#include <cassert>
#include "platform.hpp"
#include "../../../core/common/sm_common.hpp"


sm::RegisterDefinitions registers;
sm::FileDefinitions files;

constexpr std::uint16_t amount_of_regs = 7;
constexpr std::uint16_t amount_of_files = 2;
constexpr std::uint8_t record_size = 208;
constexpr std::uint8_t address = 1;

PlatformSupport platform_support;

int main(int argc, char* argv[])
{
    (void)(argc);
    (void)(argv);
    assert(amount_of_regs == registers.getSize());
    assert(amount_of_files == files.getSize());
    std::string path = "dev/null";
    
    platform_support.setPath(path);

    sm::DataNode<amount_of_regs, amount_of_files, record_size,DesktopCom,DesktopTimer> data_node(address);

    data_node.start();
    data_node.loop();
}
