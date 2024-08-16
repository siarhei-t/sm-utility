/**
 * @file sm_error.cpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../inc/sm_error.hpp"
#include <cstring>

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

            case sm::ClientErrors::bad_crc:
                return "crc check error";

            case sm::ClientErrors::timeout:
                return "no response from server, conenction timeout";

            case sm::ClientErrors::server_exception:
                return "the server did not process the command and returned an "
                       "exception";

            case sm::ClientErrors::server_not_connected:
                return "the server is not connected";
            
            case sm::ClientErrors::file_buffer_is_empty:
                return "internal file buffer is empty";

            case sm::ClientErrors::internal:
                return "internal logic error";

            default:
                return "unknown error";
        }
    }
};
} // namespace

namespace sm
{
const std::error_category& sm_category()
{
    static sm_category_impl obj;
    return obj;
}
} // namespace sm
