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
            void fileDelete();
            
            std::uint16_t getRecordLength(const int index) const;
        
        private:
            std::unique_ptr<std::uint8_t> data;
            size_t file_size = 0;
            std::uint16_t num_of_records = 0;
            std::uint16_t counter = 0;
            std::uint8_t  record_size = 0;
            std::uint16_t getNumOfRecords(const size_t file_size) const;
    };
}

#endif //SM_FILE_H
