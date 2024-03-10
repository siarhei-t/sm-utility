/**
 * @file sm_file.cpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sm_file.hpp"

namespace sm
{
    void File::fileDelete()
    {
        data.reset();
        num_of_records = 0;
        record_size    = 0;
        counter        = 0;
        file_size      = 0;
    }

    std::uint16_t File::getNumOfRecords(const size_t file_size) const
    {
        std::uint16_t num_of_records = 0;
        if((record_size > 0) && (file_size > 0))
        {
            num_of_records = (file_size % record_size)?((file_size/record_size) + 1):(file_size/record_size);
            if(num_of_records > modbus::max_num_of_records){num_of_records = 0;}
        }
        return num_of_records; 
    }
    
    std::uint16_t File::getRecordLength(const int index) const
    {
        std::uint16_t length = 0;
        if((index < num_of_records) && (index >= 0))
        {
            if((index + 1) == num_of_records)
            {
                length = (file_size == record_size)?(file_size):(file_size % record_size);
            }
            else{length = record_size;}
        }
        else{length = 0;}
        return length;
    }

}


