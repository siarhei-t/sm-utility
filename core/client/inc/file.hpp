/**
 * @file file.hpp
 *
 * @brief header for file.cpp
 *
 */

#ifndef SM_FILE_H
#define SM_FILE_H

#include <cstdint>
#include <memory>
#include <vector>

namespace sm
{

class File
{

public:
    void flush();
    bool setupRead(const std::uint16_t id, const size_t file_size, const std::uint8_t record_size);
    bool setupWriteFromDrive(const std::uint16_t id, const std::string path_to_file, const std::uint8_t record_size);
    bool setupWriteFromMemory(const std::uint16_t id, const std::vector<std::uint8_t>& file_data, const std::uint8_t record_size);
    bool loadRecordFromMessage(const std::vector<std::uint8_t>& message);
    bool isFileReady() const { return ready; }
    size_t getFileSize(const std::string path_to_file) const;
    std::uint16_t getActualRecordLength(const int index) const;
    std::uint8_t* getData() const { return data.get(); }
    std::uint16_t getId() const { return id; }
    std::uint16_t getNumOfRecords() const { return num_of_records; };

private:
    std::unique_ptr<std::uint8_t[]> data;
    size_t file_size = 0;
    std::uint16_t num_of_records = 0;
    std::uint16_t counter = 0;
    std::uint16_t id = 0;
    std::uint8_t record_size = 0;
    bool ready = false;
    std::uint16_t calcNumOfRecords(const size_t file_size) const;
};

} // namespace sm

#endif // SM_FILE_H
