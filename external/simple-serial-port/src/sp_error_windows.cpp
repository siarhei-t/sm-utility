/**
 * @file sp_error_windows.cpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sp_error.hpp"

namespace
{
    class sp_category_impl : public std::error_category
    {
        const char* name() const noexcept override { return "windows serial"; }

        std::string message(int condition) const override
        {
            return std::system_category().message(condition);
        }
    };
}

namespace sp
{
    const std::error_category& sp_category()
    {
        static sp_category_impl obj;
        return obj;
    }
}



