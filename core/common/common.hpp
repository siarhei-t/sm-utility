/**
 * @file common.hpp
 *
 * @brief TBD
 *
 */

#ifndef SM_COMMON_HPP
#define SM_COMMON_HPP

#include <cstddef>
#include <cstdint>

namespace sm
{

enum class ServerCommands : std::uint16_t
{
    read_file = 1,
    write_file = 2,
    delete_file = 3
};

enum class ServerResponces : std::uint16_t
{
    unknown = 0,
    success = 1,
    timeout = 2,
    unspecified_error = 255
};

enum class RegisterDefinitions : std::uint16_t
{
    record_counter = 1,
    status = 2,
    control = 3,
    value = 4,
    _count,
};

enum class FileDefinitions : std::uint16_t
{
    metadata = 1,
    application = 2,
    _count,
};

template <typename type>
constexpr std::uint16_t toU16(type value)
{
    return static_cast<std::uint16_t>(value);
}

constexpr std::uint16_t first_register = static_cast<std::uint16_t>(RegisterDefinitions::record_counter);
constexpr std::uint16_t registers_count = static_cast<std::uint16_t>(RegisterDefinitions::_count);
constexpr std::uint32_t protocol_version = 1;
constexpr size_t info_max_size = 64;

struct alignas(4) ServerMetaData
{
    char info[info_max_size];
    std::uint32_t flash_size;
    std::uint32_t protocol_version;
    std::uint16_t record_size;
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
