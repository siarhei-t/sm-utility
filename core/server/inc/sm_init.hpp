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
#include "sm_server.hpp"

namespace sm
{

class ServerInitializer
{
public:
    void initModbusServer(ServerResources& resources, ModbusServer& server, BufferControl& buffer);

private:
    void initResources(ServerResources& resources);
    void initServer(ModbusServer& server);
    void initBuffer(BufferControl& buffer);
};

} // namespace sm

#endif // SM_INIT_HPP
