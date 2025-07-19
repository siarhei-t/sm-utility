/**
 * @file sm_init.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sm_logic.hpp"

namespace sm
{

BufferControl* local_buffer_control = nullptr;
std::uint8_t local_record_size = 0;

void file_control(const RegisterInfo& info)
{
    switch (info.value)
    {
        case ServerCommands::file_read_prepare:
            local_buffer_control->setSize(modbus::request_read_file_pdu_size + modbus::address_size + modbus::crc_size);
            break;

        case ServerCommands::file_write_prepare:
            local_buffer_control->setSize(modbus::request_read_file_pdu_size + modbus::address_size + modbus::crc_size + local_record_size);
            break;

        default:
            break;
    }
}

void set_record_counter(const RegisterInfo& info) { local_buffer_control->setRecordCounter(info.value); }

ServerLogic::ServerLogic(BufferControl* buffer_control)
{
    local_buffer_control = buffer_control;
    local_buffer_control->setSize(default_buffer_size);
}

void ServerLogic::initModbusServer(ServerResources& resources)
{
    initResources(resources);
    local_record_size = resources.getRecordSize();
}

void ServerLogic::initResources(ServerResources& resources)
{
    RegisterInfo reg;
    FileInfo file;

    // record control register
    reg.attributes.property_read = true;
    reg.value = resources.getRecordSize();
    resources.setRegister(reg, RegisterDefinitions::record_size);
    reg = RegisterInfo();
    // file control register
    reg.attributes.property_write = true;
    reg.callback = file_control;
    resources.setRegister(reg, RegisterDefinitions::file_control);
    reg = RegisterInfo();
    // record counter register
    reg.attributes.property_write = true;
    reg.callback = set_record_counter;
    resources.setRegister(reg, RegisterDefinitions::record_counter);
    reg = RegisterInfo();
}

} // namespace sm
