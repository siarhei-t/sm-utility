/**
 * @file common.hpp
 *
 * @brief
 *
 */

#ifndef SM_COMMON_HPP
#define SM_COMMON_HPP

#include <cstdint>

namespace sm
{

enum class ServerCommands : std::uint16_t
{
    file_read_prepare = 1,
    file_write_prepare = 2,
    app_erase_request = 3
};

enum class RegisterDefinitions : std::uint16_t
{
    record_size = 0,
    control = 1,
    record_counter = 2,
    status = 3,
    _count,
};

enum class FileDefinitions : std::uint16_t
{
    application = 1,
    metadata = 2,
    _count,
};

template <typename type>
constexpr std::uint16_t toU16(type value)
{
    return static_cast<std::uint16_t>(value);
}

constexpr std::uint16_t files_offset = 1;
constexpr std::uint16_t registers_count = static_cast<std::uint16_t>(RegisterDefinitions::_count);
constexpr std::uint16_t files_count = static_cast<std::uint16_t>(FileDefinitions::_count) - files_offset;

struct ServerMetaData
{
    char boot_version[16];
    uint32_t flash_available;
};

inline std::uint16_t extract_half_word_le(const std::uint8_t* data)
{
    std::uint16_t half_word = data[0];
    half_word |= static_cast<std::uint16_t>(data[1]) << 8;
    return half_word;
}

inline void insert_half_word_le(std::uint8_t* data, const std::uint16_t half_word)
{
    data[0] = static_cast<std::uint8_t>(half_word);
    data[1] = static_cast<std::uint8_t>((half_word >> 8));
}

inline std::uint16_t extract_half_word_be(const std::uint8_t* data)
{
    std::uint16_t half_word = data[1];
    half_word |= static_cast<std::uint16_t>(data[0]) << 8;
    return half_word;
}

inline void insert_half_word_be(std::uint8_t* data, const std::uint16_t half_word)
{
    data[1] = static_cast<std::uint8_t>(half_word);
    data[0] = static_cast<std::uint8_t>((half_word >> 8));
}

} // namespace sm

#endif // SM_COMMON_HPP
