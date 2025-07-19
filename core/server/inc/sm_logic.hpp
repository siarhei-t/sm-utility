/**
 * @file sm_init.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_INIT_HPP
#define SM_INIT_HPP

#include "sm_resources.hpp"

namespace sm
{

class ServerLogic
{

public:
    ServerLogic(BufferControl* buffer_control);
    void initModbusServer(ServerResources& resources);

private:
    void initResources(ServerResources& resources);
};

} // namespace sm

#endif // SM_INIT_HPP
