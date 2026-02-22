/**
 * @file init.cpp
 *
 * @brief
 *
 */

#include "../inc/init.hpp"

namespace sm
{

void ServerLogic::init(ServerResources& resources)
{
    RegisterInfo reg;
    FileInfo file;
    // record control register
    reg.attributes.property_read = true;
    reg.value = resources.getRecordSize();
    resources.setRegister(reg, toU16<RegisterDefinitions>(RegisterDefinitions::record_size));
    reg = RegisterInfo();
    // file control register
    reg.attributes.property_write = true;
    reg.callback = ServerLogic::control;
    resources.setRegister(reg, toU16<RegisterDefinitions>(RegisterDefinitions::control));
    reg = RegisterInfo();
    // record counter register
    reg.attributes.property_write = true;
    reg.callback = ServerLogic::setRecordCounter;
    resources.setRegister(reg, toU16<RegisterDefinitions>(RegisterDefinitions::record_counter));
    reg = RegisterInfo();
}

void ServerLogic::control(const RegisterInfo& info, BufferControl& buffer_control)
{

    switch (info.value)
    {
        case toU16<ServerCommands>(ServerCommands::file_read_prepare):
            buffer_control.setBufferSize(modbus::request_read_file_pdu_size + modbus::address_size + modbus::crc_size);
            break;

        case toU16<ServerCommands>(ServerCommands::file_write_prepare):
            buffer_control.setBufferSize(modbus::request_read_file_pdu_size + modbus::address_size + modbus::crc_size + buffer_control.getRecordSize());
            break;

        default:
            break;
    }
}

void ServerLogic::setRecordCounter(const RegisterInfo& info, BufferControl& buffer_control) { buffer_control.setRecordCounter(info.value); }

} // namespace sm
