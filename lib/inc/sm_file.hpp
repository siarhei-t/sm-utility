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

#include <memory>
#include <fstream>
#include "../inc/sm_error.hpp"
#include "../inc/sm_modbus.hpp"

namespace sm
{
    class File
    {
        public:
            /// @brief delete file, release buffer
            void fileDelete();
            /// @brief prepare instance for file reading from the server
            /// @param file_size file size to read
            /// @return true in case of success
            bool fileReadSetup(const size_t file_size, const std::uint8_t record_size);
            /// @brief prepare instance for file sending to the server
            /// @param path_to_file path to file on the disk
            /// @return true in case of success
            bool fileWriteSetup(const std::string path_to_file, const std::uint8_t record_size);
            /// @brief get actual record size based on file_size, num_of_records and record_size
            /// @param index record index in file
            /// @return record size in bytes
            std::uint16_t getActualRecordLength(const int index) const;
            
            std::uint16_t getNumOfRecords() const {return num_of_records;};
        
        private:
            std::unique_ptr<std::uint8_t> data;
            size_t file_size = 0;
            std::uint16_t num_of_records = 0;
            std::uint16_t counter = 0;
            std::uint8_t  record_size = 0;
            /// @brief get num of records in file
            /// @param file_size file size in bytes
            /// @return expected number of records
            std::uint16_t calcNumOfRecords(const size_t file_size) const;
    };
}

#endif //SM_FILE_H
