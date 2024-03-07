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
        size           = 0;
    }
}


