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

/**
 * @brief Base CRTP bootloader class template.
 *
 * The `Bootloader` class provides a generic structure for initializing
 * and controlling the boot process in embedded systems. Platform-specific
 * behavior must be implemented in the derived class (`Impl`) using CRTP.
 *
 * The bootloader performs initialization of memory pages, verifies
 * application integrity, and either starts the main application or remains
 * in bootloader mode depending on the platform and application state.
 *
 * @tparam Impl Derived class implementing hardware-specific or system-level functions.
 *
 * ### Required methods in `Impl`:
 * - `ServicePage* configureServicePageAddress(std::uintptr_t address);`
 * - `ApplicationHeader* configureApplicationAddress(std::uintptr_t address);`
 * - `bool checkStayIn();` — determines whether to remain in bootloader mode.
 * - `void appStart(void* app_entry);` — starts the main application.
 */
template <class Impl>
class Bootloader
{
    /**
     * @brief Initializes the bootloader and determines the boot path.
     *
     * - Configures service and application memory pages.
     * - Verifies the integrity and presence of the main application.
     * - If verification passes, initializes boot metadata and remains in bootloader mode.
     * - Otherwise, jumps to the application entry point.
     */
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
    ServerMetaData server_metadata;                ///< Metadata structure describing bootloader state and configuration.
    ServicePage* service_page = nullptr;           ///< Pointer to the service page structure.
    ApplicationHeader* application_page = nullptr; ///< Pointer to the application header in flash memory.

    /**
     * @brief Verifies application validity and integrity.
     *
     * Performs a basic check based on the service page information.
     * More advanced integrity checks (e.g. CRC, signature verification)
     * can be implemented later.
     *
     * @return true if the application exists and passes basic validation.
     */
    bool verifyApplication() const
    {
        // normal integrity check to be added
        return service_page->app_exist;
    }
};

} // namespace sm

#endif // SM_BOOT_HPP
