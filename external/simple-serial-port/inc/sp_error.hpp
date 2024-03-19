/**
 * @file sp_error.hpp
 *
 * @brief custom system error code generator
 *
 * @author Siarhei Tatarchanka
 *
 */

#ifndef SP_ERROR_H
#define SP_ERROR_H

#include <system_error>

namespace sp
{
const std::error_category& sp_category();

inline std::error_code make_error_code(int error) noexcept
{
    return std::error_code(error, sp_category());
}
} // namespace sp

#endif // SP_ERROR_H
