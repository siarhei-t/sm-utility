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

class ServerCommands
{
public:
    static constexpr std::uint16_t file_read_prepare = 1;
    static constexpr std::uint16_t file_write_prepare = 2;
};

class RegisterDefinitions
{
public:
    static constexpr std::uint16_t record_size = 0;
    static constexpr std::uint16_t file_control = 1;
    static constexpr std::uint16_t record_counter = 2;
    static constexpr std::uint16_t prepare_to_update = 3;
    static constexpr std::uint16_t app_erase = 4;
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
