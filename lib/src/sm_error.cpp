/**
 * @file sm_error.cpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <cstring>
#include "../inc/sm_error.hpp"

namespace
{
    class sm_category_impl : public std::error_category
    {
        const char* name() const noexcept override { return "sm client"; }

        std::string message(int condition) const override
        {
            sm::ClientErrors error = static_cast<sm::ClientErrors>(condition);          
            switch (error)
            {
                case sm::ClientErrors::no_error: 
                    return "success";
                
                case sm::ClientErrors::crc_error: 
                    return "crc check error";

                default: return "unknown error";
            }
        }
    };
}

namespace sm
{
    const std::error_category& sm_category()
    {
        static sm_category_impl obj;
        return obj;
    }
}



