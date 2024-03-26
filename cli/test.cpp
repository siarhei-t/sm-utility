/**
 * @file sm_utility.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <iostream>
#include "../inc/sm_client.hpp"

const std::string interactive_text = "program started in interactive mode, type help for available commands \n";
const std::string error_text = "unsupported command passed, exit... \n";
const std::string input_start = ">";
const std::string help_text = "help";
const std::string exit_text = "exit";
const std::string scanport_text = "scanport";
const std::string status_text = "status";

enum class Commands
{
    unknown,
    help,
    exit,
    scanport,
    status
};

//struct used to store server data
sm::ServerData server_data;
//serial port address in system
std::string port = "NULL";
//server address
std::uint8_t address = 0;

static void print_devices(std::vector<std::string>& devices);
static void print_help();
static void print_status();
static Commands process_cmd(std::string& str);

static void print_devices(std::vector<std::string>& devices)
{
    std::cout<<"Available serial ports: \n";
    for(std::string& device : devices)
    {
        std::cout<<device<<"\n";
    }
}

void print_help()
{
    std::cout<<"Available commands : \n\n"
            <<help_text<<" - used for help text output; \n\n"
            <<exit_text<<" - used to exit from program; \n\n"
            <<status_text<<" - print actual client status; \n\n"
            <<scanport_text<<" - used for scan for available serial ports in system; \n\n"
            ;
}

void print_status()
{
    std::cout<<"used port : "<<port<<"\n";
    if(server_data.info.status == sm::ServerStatus::Available)
    {
        std::cout<<"connected to server with id  : "<<server_data.info.addr<<"\n";
    }
    else
    {
        std::cout<<"server not connected.\n";
    }
}

int main(int argc, char* argv[])
{
    //instance used to search for available serial ports
    sp::SerialDevice serial_device;
    //main client instance
    //sm::Client client;
    //vector with available serial ports
    std::vector<std::string> devices;
    
    if(argc == 1)
    {
        std::cout<<interactive_text;
        while(true)
        {
            std::string str;
            std::cout<<input_start;
            std::getline(std::cin, str);
            Commands cmd = process_cmd(str);
            switch(cmd)
            {
                case Commands::help:
                    print_help();
                    break;
                
                case Commands::status:
                    print_status();
                    break;

                case Commands::scanport:
                    serial_device.updateAvailableDevices();
                    devices = serial_device.getListOfAvailableDevices();
                    print_devices(devices);
                    break;

                case Commands::exit:
                    std::cout<<"program stopped, exit.\n";
                    return 0;
                
                case Commands::unknown:
                default:    
                    std::cout<<error_text;
                    return 0;
            }
        }
    }
    return 0;

    
    /*
    std::error_code error;
    sp::PortConfig config;
    config.baudrate = sp::PortBaudRate::BD_19200;
    config.timeout_ms = 2000;

    error = client.start(port);
    if(error)
    {
        std::printf("failed to start client. \n"); 
        std::cout<<"error: "<<error.message()<<"\n";
        return 0;
    }
    error = client.configure(config);
    if(error)
    {
        std::printf("failed to configure client. \n");
        std::cout<<"error: "<<error.message()<<"\n";
        return 0;
    }
    error = client.connect(address);
    if(error)
    {
        std::printf("failed to conect to server. \n");
        std::cout<<"error: "<<error.message()<<"\n";
        return 0;
    }

    std::printf("connected to server with id : %d \n",address);
    client.getServerData(server_data);
    std::printf("device name   : %s \n",server_data.data.boot_name);
    std::printf("boot version  : %s \n",server_data.data.boot_version);
    std::printf("available ROM : %d KB \n",server_data.data.available_rom/1024);
    
    error = client.uploadApp("test.bin");
    if(error)
    {
        std::printf("failed to upload firmware. \n");
        std::cout<<"error: "<<error.message()<<"\n";
        return 0;
    }
    /*
    std::printf("app erase request... \n");
    error = client.eraseApp(); 
    if(error)
    {
        std::printf("failed to erase app on server. \n");
        std::cout<<"error: "<<error.message()<<"\n";
        return 0;
    }
    */
    return 0;
}

Commands process_cmd(std::string& str) 
{
    Commands cmd = Commands::unknown;
    if(str == help_text)
    {
        cmd = Commands::help;
    }
    if(str == exit_text)
    {
        cmd = Commands::exit;
    }
    if(str == scanport_text)
    {
        cmd = Commands::scanport;
    }
    if(str == status_text)
    {
        cmd = Commands::status;
    }
    return cmd; 
}
