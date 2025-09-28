/**
 * @file boot.hpp
 *
 * @brief
 *
 */

#ifndef SM_BOOT_HPP
#define SM_BOOT_HPP

#include "../../common/common.hpp"
#include <cstdint>

namespace sm
{

struct ApplicationHeader
{
    std::uint8_t signature[64];
    std::uint32_t signature_type;
    std::uint32_t file_size;
};

struct alignas(4) ServicePage
{
    bool app_exist;
    uint32_t app_start_address;
};

template <class Impl>
class Bootloader
{
    void init() {}

public:
private:
    ServerMetaData server_metadata;
};

} // namespace sm

#endif // SM_BOOT_HPP
