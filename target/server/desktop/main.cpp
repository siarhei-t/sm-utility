/**
 * @file main.cpp
 *
 * @brief 
 *
 * @author
 *
 */


#include "platform.hpp"

#include "../../../core/common/sm_common.hpp"
#include <cassert>

sm::RegisterDefinitions registers;
sm::FileDefinitions files;

constexpr std::uint16_t amount_of_regs = 7;
constexpr std::uint16_t amount_of_files = 2;
constexpr std::uint8_t record_size = 208;
constexpr std::uint8_t address = 1;

int main(int argc, char* argv[])
{
    (void)(argc);
    (void)(argv);
    assert(amount_of_regs == registers.getSize());
    assert(amount_of_files == files.getSize());

    sm::DataNode<amount_of_regs, amount_of_files, record_size> data_node(1);
    
    data_node.start();
    data_node.loop();
}
