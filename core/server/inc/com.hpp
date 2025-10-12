#ifndef SM_COM_HPP
#define SM_COM_HPP

#include <atomic>

namespace sm
{
/**
 * @brief Base CRTP communication interface class.
 *
 * The `Com` class provides a generic, platform-independent communication interface
 * and expects the derived class (`Impl`) to implement the platform-specific logic.
 * This pattern allows zero-overhead abstraction using the Curiously Recurring Template Pattern (CRTP).
 *
 * @tparam Impl Derived class implementing platform-specific communication methods.
 *
 * ### Required methods in `Impl`:
 * - `bool platformInit();` — initialize the communication interface.
 * - `void platformReadData(std::uint8_t* data, size_t amount);` — perform a platform-specific read.
 * - `void platformSendData(std::uint8_t* data, size_t amount);` — perform a platform-specific send.
 * - `void platformFlush();` — flush or clear the communication buffer.
 */
template <class Impl>
class Com
{
public:
    /**
     * @brief Initializes the communication interface.
     *
     * Calls the derived implementation `platformInit()`
     * and stores the configuration state.
     */
    void init() { configured = static_cast<Impl*>(this)->platformInit(); }

    /**
     * @brief Reads data from the communication interface.
     *
     * Calls `platformReadData()` if the interface is configured.
     * Marks the interface as not ready before starting the read operation.
     *
     * @param data Pointer to the destination buffer.
     * @param amount Number of bytes to read.
     */
    void readData(std::uint8_t* data, const size_t amount)
    {
        if (isConfigured())
        {
            ready.store(false, std::memory_order_relaxed);
            static_cast<Impl*>(this)->platformReadData(data, amount);
        }
    }

    /**
     * @brief Sends data through the communication interface.
     *
     * Calls `platformSendData()` if the interface is configured.
     *
     * @param data Pointer to the data buffer.
     * @param amount Number of bytes to send.
     */
    void sendData(std::uint8_t* data, const size_t amount)
    {
        if (isConfigured())
        {
            static_cast<Impl*>(this)->platformSendData(data, amount);
        }
    }

    /**
     * @brief Flushes the communication interface.
     *
     * Calls `platformFlush()` if the interface is configured.
     */
    void flush()
    {
        if (isConfigured())
        {
            static_cast<Impl*>(this)->platformFlush();
        }
    }

    /**
     * @brief Checks if the communication interface is configured.
     * @return true if initialized successfully, false otherwise.
     */
    [[nodiscard]] bool isConfigured() const { return configured; }

    /**
     * @brief Checks if the interface is ready for a new operation.
     * @return true if ready, false otherwise.
     */
    [[nodiscard]] bool isReady() const { return ready.load(std::memory_order_acquire); }

    /**
     * @brief Checks if the interface is currently busy.
     * @return true if busy, false otherwise.
     */
    [[nodiscard]] bool isBusy() const { return busy.load(std::memory_order_acquire); }

    /**
     * @brief Marks the communication interface as ready.
     *
     * Typically called by platform-specific code when a read or write
     * operation is completed.
     */
    void setReady() { ready.store(true, std::memory_order_release); }

    /**
     * @brief Marks the communication interface as busy.
     *
     * Can be used by the platform layer to indicate
     * that a transfer or processing is in progress.
     */
    void setBusy() { busy.store(true, std::memory_order_release); }

private:
    bool configured = false;        ///< Indicates whether the interface has been initialized.
    std::atomic<bool> ready{false}; ///< Indicates readiness for a new operation.
    std::atomic<bool> busy{false};  ///< Indicates an ongoing operation.
};

} // namespace sm

#endif // SM_COM_HPP
