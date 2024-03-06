/**
 * @file sp_error_linux.cpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <errno.h>
#include <cstring>
#include "../inc/sp_error.hpp"

namespace
{
    class sp_category_impl : public std::error_category
    {
        const char* name() const noexcept override { return "linux serial"; }

        std::string message(int condition) const override
        {
            return std::string(strerror(condition));
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



