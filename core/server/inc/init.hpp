/**
 * @file init.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_INIT_HPP
#define SM_INIT_HPP

#include "resources.hpp"

namespace sm
{

class ServerLogic
{

public:
    ServerLogic() {}
    void init(ServerResources& resources);

private:
    static void control(const RegisterInfo& info, BufferControl& buffer_control);
    static void setRecordCounter(const RegisterInfo& info, BufferControl& buffer_control);
};

} // namespace sm

#endif // SM_INIT_HPP
