/**
 * @file sm_file.cpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */


#include "../inc/sm_file.hpp"
#include <iostream>


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

    bool File::fileReadSetup(const size_t file_size, const std::uint8_t record_size)
    {
        if(data){fileDelete();}

        data =  std::unique_ptr<std::uint8_t>(new std::uint8_t(file_size));    
        this->file_size = file_size;
        this->record_size = record_size;
        num_of_records = calcNumOfRecords(file_size);
        if(!data){return false;}
        else{return true;}
    }

    bool File::fileWriteSetup(const std::string path_to_file, const std::uint8_t record_size)
    {
        return false;
    }

    std::uint16_t File::calcNumOfRecords(const size_t file_size) const
    {
        std::uint16_t num_of_records = 0;
        if((record_size > 0) && (file_size > 0))
        {
            num_of_records = (file_size % record_size)?((file_size/record_size) + 1):(file_size/record_size);
            if(num_of_records > modbus::max_num_of_records){num_of_records = 0;}
        }
        return num_of_records; 
    }
    
    std::uint16_t File::getActualRecordLength(const int index) const
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


