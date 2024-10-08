/**
 * @file sm_common.hpp
 *
 * @brief
 *
 * @author
 *
 */

#ifndef SM_COMMON_HPP
#define SM_COMMON_HPP

#include <cstdint>

namespace sm
{

class RegisterDefinitions
{
public:
    static constexpr std::uint16_t file_control = 0;
    static constexpr std::uint16_t prepare_to_update = 1;
    static constexpr std::uint16_t app_erase = 2;
    static constexpr std::uint16_t record_size = 3;
    static constexpr std::uint16_t record_counter = 4;
    static constexpr std::uint16_t status = 5;
    static constexpr std::uint16_t gateway_buffer_size = 6;

    static constexpr std::uint16_t getSize() { return size; }

private:
    static constexpr std::uint16_t size = 7;
};

class FileDefinitions
{
public:
    static constexpr std::uint16_t application = 1;
    static constexpr std::uint16_t metadata = 2;

    static constexpr std::uint16_t getSize() { return size; }

private:
    static constexpr std::uint16_t size = 2;
};

} // namespace sm

#endif // SM_COMMON_HPP
