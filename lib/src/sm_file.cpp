/**
 * @file sm_file.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sm_file.hpp"
#include "sm_file.hpp"
#include <cstring>
#include <iostream>

namespace sm
{

size_t File::getFileSize(const std::string path_to_file) const
{
    size_t length = 0;
    std::ifstream tmp(path_to_file, std::ifstream::binary);
    if (tmp)
    {
        tmp.seekg(0, tmp.end);
        length = tmp.tellg();
        tmp.seekg(0, tmp.beg);
        tmp.close();
    }
    return length;
}

void File::fileDelete()
{
    data.reset();
    num_of_records = 0;
    record_size = 0;
    counter = 0;
    file_size = 0;
    ready = false;
}

bool File::fileReadSetup(const size_t file_size, const std::uint8_t record_size)
{
    if (data)
    {
        fileDelete();
    }
    data = std::make_unique<std::uint8_t[]>(file_size);
    this->file_size = file_size;
    this->record_size = record_size;
    num_of_records = calcNumOfRecords(file_size);
    return data != nullptr;
}

bool File::fileWriteSetup(const std::string path_to_file, const std::uint8_t record_size)
{
    if (data)
    {
        fileDelete();
    }
    size_t length = getFileSize(path_to_file);
    if (length > 0)
    {
        std::ifstream tmp(path_to_file, std::ifstream::binary);
        if (tmp)
        {
            data = std::make_unique<std::uint8_t[]>(length);
            // load all file to RAM buffer at one time
            tmp.read(reinterpret_cast<char*>(data.get()), length);
            if (tmp)
            {
                file_size = length;
            }
            else
            {
                file_size = tmp.gcount();
            }
            tmp.close();
        }
        if (file_size == length)
        {
            this->record_size = record_size;
            num_of_records = calcNumOfRecords(file_size);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool File::getRecordFromMessage(const std::vector<std::uint8_t>& message)
{
    const std::uint8_t data_idx = 5;
    const std::uint8_t data_size = message[2] - 1;
    const int record_idx = counter * record_size;

    if (static_cast<size_t>(record_idx + data_size) <= file_size)
    {
        std::copy(message.data() + data_idx, message.data() + data_idx + data_size, data.get() + record_idx);
        ++counter;
        if (counter == num_of_records)
        {
            ready = true;
        }
        return true;
    }
    else
    {
        return false;
    }
}

std::uint16_t File::calcNumOfRecords(const size_t file_size) const
{
    std::uint16_t num_of_records = 0;
    if ((record_size > 0) && (file_size > 0))
    {
        num_of_records = (file_size % record_size) ? ((file_size / record_size) + 1) : (file_size / record_size);
        if (num_of_records > modbus::max_num_of_records)
        {
            num_of_records = 0;
        }
    }
    return num_of_records;
}

std::uint16_t File::getActualRecordLength(const int index) const
{
    std::uint16_t length = 0;
    if ((index < num_of_records) && (index >= 0))
    {
        if ((index + 1) == num_of_records)
        {
            length = (file_size == record_size) ? (file_size) : (file_size % record_size);
        }
        else
        {
            length = record_size;
        }
    }
    else
    {
        length = 0;
    }
    return length;
}

} // namespace sm
