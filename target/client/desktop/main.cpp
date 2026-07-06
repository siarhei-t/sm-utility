/**
 * @file sm_utility.hpp
 *
 * @brief
 *
 * @author Siarhei Tatarchanka
 *
 */

#include "../../../core/client/inc/client.hpp"
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

enum class Command
{
    None,
    Help,
    Ping,
    Read,
    Info,
    Upload
};

struct Options
{
    Command command = Command::None;

    std::optional<std::string> address;
    std::optional<std::string> device_path;
    std::optional<std::string> file_path;
};

[[noreturn]] void throw_error(const std::string& msg) { throw std::runtime_error(msg); }

void print_help(const char* prog)
{
    std::cout << "Usage:\n"
                 "  "
              << prog
              << " help\n"
                 "  "
              << prog
              << " ping   --address <addr> --device <path>\n"
                 "  "
              << prog
              << " read   --address <addr> --device <path>\n"
                 "  "
              << prog
              << " info   --address <addr> --device <path>\n"
                 "  "
              << prog
              << " upload --address <addr> --device <path> --file <file>\n\n"
                 "Options:\n"
                 "  --address <addr>      Device network address\n"
                 "  --device  <path>      Path to device (e.g. /dev/ttyUSB0)\n"
                 "  --file    <file>      File to upload (upload only)\n";
}

Command parse_command(std::string_view cmd)
{
    if (cmd == "help")
        return Command::Help;
    if (cmd == "ping")
        return Command::Ping;
    if (cmd == "read")
        return Command::Read;
    if (cmd == "info")
        return Command::Info;
    if (cmd == "upload")
        return Command::Upload;

    throw_error("Unknown command: " + std::string(cmd));
}

Options parse_args(int argc, char* argv[])
{
    if (argc < 2)
        throw_error("No command specified");

    Options opts;
    opts.command = parse_command(argv[1]);

    for (int i = 2; i < argc; ++i)
    {
        std::string_view arg = argv[i];

        if (arg == "--address")
        {
            if (++i >= argc)
                throw_error("Missing value for --address");

            opts.address = argv[i];
        }
        else if (arg == "--device")
        {
            if (++i >= argc)
                throw_error("Missing value for --device");

            opts.device_path = argv[i];
        }
        else if (arg == "--file")
        {
            if (++i >= argc)
                throw_error("Missing value for --file");

            opts.file_path = argv[i];
        }
        else
        {
            throw_error("Unknown option: " + std::string(arg));
        }
    }

    return opts;
}

void validate(const Options& opts)
{
    if (opts.command == Command::Help)
        return;

    if (!opts.address)
        throw_error("--address is required");

    if (!opts.device_path)
        throw_error("--device is required");

    if (opts.command == Command::Upload && !opts.file_path)
        throw_error("--file is required for upload command");

    if (opts.command != Command::Upload && opts.file_path)
        throw_error("--file is only valid for upload command");
}

int main(int argc, char* argv[])
{

    if (argc < 3)
    {
        std::printf("incorrect arguments list passed, exit...\n");
        return 0;
    }
    sm::ModbusClient client;
    sp::PortConfig config;
    std::string path_to_port = argv[1];
    std::string address_str = argv[2];
    std::uint8_t address;

    try
    {
        auto number = std::stoi(address_str);
        if (number > modbus::max_rtu_address)
        {
            std::cout << "out of range address passed, exit...\n";
            return 0;
        }
        else if (number < modbus::min_rtu_address)
        {
            std::cout << "broadcast is not supported, exit...\n";
            return 0;
        }
        else
        {
            address = static_cast<std::uint8_t>(number);
        }
    }
    catch (std::invalid_argument const& ex)
    {
        std::cout << "invalid argument passed, exit...\n";
        return 0;
    }

    // FIXME: fix it to read config from command line int the future
    config.baudrate = sp::PortBaudRate::BD_57600;
    config.timeout_ms = 2000;

    auto error_code = client.start(path_to_port);
    if (error_code)
    {
        std::cout << "failed to start client. \n";
        std::cout << "error: " << error_code.message() << "\n";
        return 0;
    }

    error_code = client.configure(config);
    if (error_code)
    {
        std::cout << "failed to configure client. \n";
        std::cout << "error: " << error_code.message() << "\n";
        return 0;
    }

    // create server instance
    client.addServer(address);

    // ping server, expected answer with modbus::Exceptions::exception_1
    error_code = client.taskPing(address);
    if (error_code)
    {
        std::cout << "error: " << error_code.message() << "\n";
    }
    std::cout << "ping success. \n";
    error_code = client.taskReadRegisters(address, modbus::holding_regs_offset, 1);
    if (error_code)
    {
        std::cout << "error: " << error_code.message() << "\n";
    }

    try
    {
        auto opts = parse_args(argc, argv);

        if (opts.command == Command::Help)
        {
            print_help(argv[0]);
            return 0;
        }

        validate(opts);

        switch (opts.command)
        {
            case Command::Ping:
                std::cout << "PING\n";
                break;

            case Command::Read:
                std::cout << "READ\n";
                break;

            case Command::Info:
                std::cout << "INFO\n";
                break;

            case Command::Upload:
                std::cout << "UPLOAD\n";
                std::cout << "File: " << *opts.file_path << "\n";
                break;

            default:
                break;
        }

        std::cout << "Address: " << *opts.address << "\n";
        std::cout << "Device:  " << *opts.device_path << "\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n\n";
        print_help(argv[0]);
        return 1;
    }

    return 0;
}
