/**
 * @file sm_resources.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sm_resources.hpp"
#include <cstddef>
#include <cstring>

namespace sm
{

bool ServerResources::getAccessToRecord(const FileService& service, FileControl& control)
{

    control.index = 0;
    control.length = 0;
    control.p_record = nullptr;

    if ((service.length > (record_size / 2)) || (service.file_id == 0) || (service.file_id > files.size()) || (service.record_id > modbus::max_num_of_records))
    {
        return false;
    }
    else
    {
        size_t index = service.file_id - 1;
        if (service.record_id > (files[index].amount_of_records - 1))
        {
            return false;
        }
        else
        {
            control.length = service.length * 2;
            control.index = index;
            control.p_record = (uint8_t*)(files[index].data.p_data) + service.record_id * record_size;
            return true;
        }
    }
}

bool ServerResources::fileOperationProcess(const int index)
{

    if (index > (files.size() - 1))
    {
        resetFileOperation();
        return false;
    }
    else
    {
        ++file_record_counter;
        if (file_record_counter >= files[index].amount_of_records)
        {
            resetFileOperation();
            if (files[index].callback != nullptr)
            {
                files[index].callback(files[index]);
            }
        }
        return true;
    }
}

bool ServerResources::writeRegister(const std::uint16_t address, const std::uint16_t value)
{
    if (address < modbus::holding_regs_offset)
    {
        return false;
    }
    const std::uint16_t offset_address = address - modbus::holding_regs_offset;
    if (offset_address > registers.size())
    {
        return false;
    }
    if (registers[offset_address].attributes.property_write)
    {
        registers[offset_address].value = value;
        if (registers[offset_address].callback != nullptr)
        {
            registers[offset_address].callback(registers[offset_address]);
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool ServerResources::readRegister(const std::uint16_t address, const std::uint16_t quantity, std::uint8_t* data, std::uint8_t& size)
{
    if (address < modbus::holding_regs_offset)
    {
        return false;
    }
    const std::uint16_t offset_address = address - modbus::holding_regs_offset;
    if (offset_address > (registers.size() - quantity))
    {
        return false;
    }
    data[0] = static_cast<std::uint8_t>((quantity * 2));
    int counter = 1;
    for (int i = 0; i < quantity; ++i)
    {
        if (registers[offset_address + i].attributes.property_read)
        {
            insert_half_word(&data[counter], registers[offset_address + i].value);
            counter += 2;
        }
        else
        {
            return false;
        }
    }
    size = data[0] + 1;
    return true;
}

bool ServerResources::writeFile(const FileService& service, const std::uint8_t* data)
{
    FileControl file_control;
    if (!getAccessToRecord(service, file_control))
    {
        return false;
    }
    if (!files[file_control.index].attributes.property_write)
    {
        return false;
    }

    if (files[file_control.index].attributes.property_flash)
    {
        if (fileWrite == nullptr)
        {
            return false;
        }
        else
        {
            bool hw_status = fileWrite(file_control.p_record, file_control.length);
            if (!hw_status)
            {
                resetFileOperation();
                return false;
            }
        }
    }
    else
    {
        std::memcpy(file_control.p_record, data, file_control.length);
    }
    return fileOperationProcess(file_control.index);
}

bool ServerResources::readFile(const FileService& service, std::uint8_t* data, std::uint8_t& size)
{
    FileControl file_control;
    if (!getAccessToRecord(service, file_control))
    {
        return false;
    }
    if (!files[file_control.index].attributes.property_read)
    {
        return false;
    }
    data[0] = file_control.length + 1;
    data[1] = file_control.length;
    data[2] = modbus::rw_file_reference;
    std::memcpy(data + 3, file_control.p_record, file_control.length);
    size = file_control.length + 3;
    return fileOperationProcess(file_control.index);
}

} // namespace sm
