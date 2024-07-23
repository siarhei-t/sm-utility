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
    /// @brief delete file, release buffer
    void fileDelete();
    /// @brief prepare instance for file reading from the server
    /// @param id file id on the server
    /// @param file_size file size to read
    /// @return true in case of success
    bool fileReadSetup(const std::uint16_t id, const size_t file_size, const std::uint8_t record_size);
    /// @brief prepare instance for file sending to the server
    /// @param id file id on the server
    /// @param path_to_file path to file on the disk
    /// @return true in case of success
    bool fileExternalWriteSetup(const std::uint16_t id, const std::string path_to_file, const std::uint8_t record_size);
    /// @brief get actual record size based on file_size, num_of_records and
    /// record_size
    /// @param index record index in file
    /// @return record size in bytes
    std::uint16_t getActualRecordLength(const int index) const;
    /// @brief get actual number of records
    /// @return number of records
    std::uint16_t getNumOfRecords() const { return num_of_records; };
    /// @brief load record from Modbus PDU
    /// @param message vector with PDU
    /// @return true in case of success
    bool getRecordFromMessage(const std::vector<std::uint8_t>& message);
    /// @brief check if file is loaded completely
    /// @return true if yes false if not
    bool isFileReady() const { return ready; }
    /// @brief get pointer to file
    /// @return pointer to buffer with file
    std::uint8_t* getData() const { return data.get(); }
    /// @brief get file id
    /// @return actual file id
    std::uint16_t getId() const { return id; }
    /// @brief calculate file size in bytes
    /// @param path_to_file path to file
    /// @return size in bytes, 0 in case of error on empty file
    size_t getFileSize(const std::string path_to_file) const;

private:
    std::unique_ptr<std::uint8_t[]> data;
    size_t file_size = 0;
    std::uint16_t num_of_records = 0;
    std::uint16_t counter = 0;
    std::uint16_t id = 0;
    std::uint8_t record_size = 0;
    bool ready = false;
    /// @brief get num of records in file
    /// @param file_size file size in bytes
    /// @return expected number of records
    std::uint16_t calcNumOfRecords(const size_t file_size) const;
};
} // namespace sm

#endif // SM_FILE_H
