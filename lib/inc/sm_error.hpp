/**
 * @file sm_error.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#ifndef SM_ERROR_H
#define SM_ERROR_H

#include <system_error>

namespace sm
{
    enum class ClientErrors
    {
        no_error,
        crc_error
    };

    const std::error_category& sm_category();
    
    inline std::error_code make_error_code(ClientErrors error) noexcept
    {
        return std::error_code(static_cast<int>(error), sm_category());
    }
}

#endif //SM_ERROR_H
