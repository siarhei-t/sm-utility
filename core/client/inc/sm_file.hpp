/**
 * @file sm_file.hpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
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
    void fileDelete();

    bool fileReadSetup(const std::uint16_t id, const size_t file_size, const std::uint8_t record_size);

    bool fileWriteSetupFromDrive(const std::uint16_t id, const std::string path_to_file, const std::uint8_t record_size);

    bool fileWriteSetupFromMemory(const std::uint16_t id, const std::vector<std::uint8_t>& file_data, const std::uint8_t record_size);

    std::uint16_t getActualRecordLength(const int index) const;

    std::uint16_t getNumOfRecords() const { return num_of_records; };

    bool getRecordFromMessage(const std::vector<std::uint8_t>& message);

    bool isFileReady() const { return ready; }

    std::uint8_t* getData() const { return data.get(); }

    std::uint16_t getId() const { return id; }

    size_t getFileSize(const std::string path_to_file) const;

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
