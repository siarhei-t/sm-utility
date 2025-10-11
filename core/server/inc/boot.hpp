/**
 * @file boot.hpp
 *
 * @brief
 *
 */

#ifndef SM_BOOT_HPP
#define SM_BOOT_HPP

#include "../../common/common.hpp"
#include "config.hpp"
#include <cassert>
#include <cstdio>

namespace sm
{

struct alignas(4) ApplicationHeader
{
    std::uint8_t signature[64];
    std::uint32_t signature_type;
    std::uint32_t file_size;
    std::uint32_t application;
};

struct alignas(4) ServicePage
{
    bool app_exist;
};

template <class Impl>
class Bootloader
{
    void init()
    {
        service_page = static_cast<Impl*>(this)->configureServicePageAddress(cfg::service_page_address);
        application_page = static_cast<Impl*>(this)->configureApplicationAddress(cfg::application_address);
        bool server_started = verifyApplication();
        server_started |= static_cast<Impl*>(this)->checkStayIn();
        if (server_started)
        {
            server_metadata.flash_available = cfg::application_size;
            (void)std::snprintf(&server_metadata.boot_version, sizeof(server_metadata.boot_version), "0.01");
        }
        else
        {
            static_cast<Impl*>(this)->appStart(application_page->application);
        }
    }

public:
private:
    ServerMetaData server_metadata;
    ServicePage* service_page = nullptr;
    ApplicationHeader* application_page = nullptr;
    bool verifyApplication() const
    {
        // normal integrity check to be added
        return service_page->app_exist;
    }
};

} // namespace sm

#endif // SM_BOOT_HPP
