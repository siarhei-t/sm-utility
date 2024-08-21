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
    bad_crc,
    timeout,
    server_exception,
    server_not_exist,
    server_not_connected,
    file_buffer_is_empty,
    max_record_length_not_configured,
    internal
};

const std::error_category& sm_category();

inline std::error_code make_error_code(ClientErrors error) noexcept { return std::error_code(static_cast<int>(error), sm_category()); }
} // namespace sm

#endif // SM_ERROR_H
