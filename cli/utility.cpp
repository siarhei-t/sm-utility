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

const std::string interactive_text = "program started in interactive mode, type help for available commands. \n";
const std::string error_text = "unsupported command passed, type help to see available commands. \n";
const std::string input_start = ">";
const std::string help_text = "help";
const std::string exit_text = "exit";
const std::string scanport_text = "scanport";
const std::string status_text = "status";
const std::string start_text = "start";
const std::string connect_text = "connect";
const std::string disconnect_text = "disconnect";
const std::string upload_text = "upload";
const std::string erase_text = "erase";
const std::string stop_text = "stop";
const std::string goapp_text = "goapp";

enum class Commands
{
    unknown,
    help,
    exit,
    scanport,
    start,
    stop,
    status,
    connect,
    disconnect,
    upload,
    erase,
    goapp
};

sm::ServerData server_data;
std::string firmware_file = "NULL";
std::string port          = "NULL";
std::uint8_t server_address = 0;
std::vector<std::string> devices;
sp::SerialDevice serial_device;
sm::Client client; 
sp::PortConfig config;

static void    print_devices();
static void     print_help();
static void     print_status();
static bool     execute_cmd(const Commands cmd);
static Commands parse_str(const std::string& str);

int main(int argc, char* argv[])
{
    (void)(argv);
    //hardcoded port parameters, 57600 bd, 2s timeout
    config.baudrate = sp::PortBaudRate::BD_115200;
    config.timeout_ms = 2000;
    
    //master chip
    client.addServer(1);
    //slave chip gatewayed through master
    client.addServer(2,1);

    if(argc == 1)
    {
        std::cout<<interactive_text;
        for (;;)
        {
            std::string str;
            std::cout<<input_start;
            std::getline(std::cin, str);
            Commands cmd = parse_str(str);
            if (!execute_cmd(cmd))
            {
                break;
            }
        }
    }
    return 0;
}

static void print_devices()
{
    std::cout<<"Available serial ports: \n";
    for(std::string& device : devices)
    {
        std::cout<<device<<"\n";
    }
}

static void print_help()
{
    std::cout<<"Available commands : \n\n"
            <<"help       - used for help text output; \n\n"
            <<"exit       - used to exit from program; \n\n"
            <<"status     - print actual client status; \n\n"
            <<"scanport   - used for scan for available serial ports in system; \n\n"
            <<"start      - start client on selected port, usage example : start COM1; \n\n"
            <<"stop       - stop client, close port; \n\n"
            <<"connect    - connect to server with passed id, usage example : connect 77; \n\n"
            <<"disconnect - disconnect from server; \n\n"
            <<"upload     - upload new firmware to the server, usage example : upload firmware.bin; \n\n"
            <<"erase      - erase firmware from server; \n\n"
            <<"goapp      - start application on server; \n\n"
            ;
}

static void print_status()
{
    std::cout<<"used port : "<<port<<"\n";
    std::printf("connected to server with id : %d \n",server_address);
    std::printf("device name   : %s \n",server_data.data.boot_name);
    std::printf("boot version  : %s \n",server_data.data.boot_version);
    std::printf("available ROM : %d KB \n",server_data.data.available_rom/1024);
    std::cout<<"app status: ";
    switch(server_data.regs[static_cast<int>(sm::ServerRegisters::boot_status)])
    {
        case 1:
            std::cout<<" not presented \n";
            break;
        case 2:
            std::cout<<" ok \n";
            break;
        case 3:
            std::cout<<" error \n";
            break;            
        case 0:
        default:
            std::cout<<" unknown \n";
            break;
    }
}

static Commands parse_str(const std::string& str) 
{    
    auto get_argv = [](std::vector<std::string>& argv, const std::string& str)
    {
        std::string arg = "";
        for (auto x : str) 
        {
            if (x == ' ')
            {
                argv.push_back(arg);
                arg = "";
            }
            else
            {
                arg = arg + x;
            }
        }
        argv.push_back(arg);
    };

    Commands cmd = Commands::unknown;
    std::vector<std::string> argv;
    get_argv(argv,str);

    if(argv[0] == help_text)
    {
        cmd = Commands::help;
    }
    if(argv[0] == exit_text)
    {
        cmd = Commands::exit;
    }
    if(argv[0] == stop_text)
    {
        cmd = Commands::stop;
    }
    if(argv[0] == scanport_text)
    {
        cmd = Commands::scanport;
    }
    if(argv[0] == status_text)
    {
        cmd = Commands::status;
    }
    if(argv[0] == erase_text)
    {
        cmd = Commands::erase;
    }
    if(argv[0] == disconnect_text)
    {
        cmd = Commands::disconnect;
    }
    if(argv[0] == goapp_text)
    {
        cmd = Commands::goapp;
    }
    if(argv[0] == connect_text)
    {
        if(argv.size() == 2)
        {
            try
            {
                server_address = std::stoi(argv[1]);
                cmd = Commands::connect;
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    }
    if(argv[0] == upload_text)
    {
        if(argv.size() == 2)
        {
            firmware_file = argv[1];
            cmd = Commands::upload;
        }
    }
    if(argv[0] == start_text)
    {
        if(argv.size() == 2)
        {
            port = argv[1];
            cmd = Commands::start;
        }
    }
    return cmd;
}

static bool execute_cmd(const Commands cmd)
{
    std::error_code error;
    switch(cmd)
    {
        case Commands::help:
            print_help();
            break;
        
        case Commands::status:
            client.getServerData(1,server_data);
            print_status();
            break;

        case Commands::scanport:
            serial_device.updateAvailableDevices();
            devices = serial_device.getListOfAvailableDevices();
            print_devices();
            break;

        case Commands::exit:
            std::cout<<"program stopped, exit.\n";
            return false;
        
        case Commands::stop:
            client.getServerData(1,server_data);
            if(server_data.info.status == sm::ServerStatus::Available)
            {
                client.disconnect();
                std::printf("disconnected from server with id : %d \n",server_address);
                server_address = 0;    
            }
            client.stop();
            port = "NULL";
            std::cout<<"client stopped.\n";
            break;

        case Commands::start:
            error = client.start(port);
            if(error)
            {
                std::cout<<"failed to start client. \n"; 
                std::cout<<"error: "<<error.message()<<"\n";
            }
            error = client.configure(config);
            if(error)
            {
                std::cout<<"failed to configure client. \n";
                std::cout<<"error: "<<error.message()<<"\n";
            }
            else
            {
                std::cout<<"client started at "<<port<<" ...\n";
            }
            break;
        
        case Commands::upload:
            error = client.uploadApp(2,firmware_file);
            if(error)
            {
                std::printf("failed to upload firmware. \n");
                std::cout<<"error: "<<error.message()<<"\n";
            }
            else
            {
                std::cout<<"new firmware uploaded.\n";
            }
            break;
        
        case Commands::disconnect:
            client.getServerData(1,server_data);
            if(server_data.info.status == sm::ServerStatus::Available)
            {
                client.disconnect();
                std::printf("disconnected from server with id : %d \n",server_address);
                server_address = 0;    
            }
            else
            {
                 std::cout<<"client is not connected.\n";
            }
            break;
        
        case Commands::erase:
            error = client.eraseApp(1);
            if(error)
            {
                std::printf("failed to erase app on server. \n");
                std::cout<<"error: "<<error.message()<<"\n";
            }
            else
            {
                std::cout<<"firmware erased.\n";
            }
            break;

        case Commands::connect:
            error = client.connect(server_address);
            if(error)
            {
                std::printf("failed to conect to server. \n");
                std::cout<<"error: "<<error.message()<<"\n";
            }
            else
            {
                client.getServerData(1,server_data);
                print_status();
            }
            break;
        case Commands::goapp:
            client.startApp(1);
            break;
            
        case Commands::unknown:
        default:    
            std::cout<<error_text;
    }
    return true;
}
