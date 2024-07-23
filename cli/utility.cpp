/**
 * @file sm_utility.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <iostream>
#include "../lib/inc/sm_client.hpp"

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

const std::string version = "0.01";

constexpr int master_address = 1;
constexpr int slave_address  = 2;

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


static void print_metadata(sm::ServerData& server_data);

int main(int argc, char* argv[])
{
    //instance of serial port scanner
    sp::SerialDevice serial_device;
    //array with master and slave servers metadata
    sm::ServerData servers[2];
    //instance of modbus client
    sm::Client client; 
    //serial port configuration
    sp::PortConfig config;

    //setup serial port initial configuration
    config.baudrate = sp::PortBaudRate::BD_115200;
    config.timeout_ms = 2000;
    if(argc != 4)
    {
        std::printf("incorrect agruments list passed, exit...\n");        
        return 0;
    }

    std::printf("\n");
    std::printf("*-------------------INFO----------------------*\n");
    std::printf("program name : EXOPULSE CU firmware update utility.\n");
    std::printf("version      : %s \n",version.c_str());
    //std::printf("*---------------------------------------------*\n");
    std::printf("*--------------CONFIGURATION------------------*\n");
    std::printf("serial port baudrate       : 115200\n");
    std::printf("master chip Modbus address : %d \n",master_address);
    std::printf("slave  chip Modbus address : %d \n",slave_address);
    std::printf("*---------------------------------------------*\n");
    // (1) check for serial ports in the system
    std::printf("scanning system serial ports...\n");
    serial_device.updateAvailableDevices();
    std::vector<std::string> devices = serial_device.getListOfAvailableDevices();
    std::cout<<"Available serial ports: ";
    for(std::string& device : devices){std::cout<<device<<" ";}
    std::printf("\n");
    std::printf("*---------------------------------------------*\n");
    std::string serial_port = argv[1];
    std::printf("trying to open serial port with name %s...\n",serial_port.c_str());
    // (2) trying to open serial port with passed address
    auto error_code = client.start(serial_port);
    if(error_code)
    {
        std::cout<<"failed to start client. \n"; 
        std::cout<<"error: "<<error_code.message()<<"\n";
        return 0;
    }
    error_code = client.configure(config);
    if(error_code)
    {
        std::cout<<"failed to configure client. \n";
        std::cout<<"error: "<<error_code.message()<<"\n";
        return 0;
    }
    else
    {
        std::cout<<"success, client started...\n";
    }
    // (3) create master and slave server instances
    client.addServer(master_address);
    client.addServer(slave_address,master_address);
    std::printf("*---------------------------------------------*\n");
    std::printf("trying to connect to master chip server...\n");
    // (4) connecting to master
    error_code = client.connect(master_address);
    if(error_code)
    {
        std::printf("failed to conect to server. \n");
        std::cout<<"error: "<<error_code.message()<<"\n";
        return 0;
    }
    else
    {
        client.getServerData(master_address,servers[0]);
        print_metadata(servers[0]);
    }
    
    std::printf("*---------------------------------------------*\n");
    std::printf("trying to connect to slave chip server...\n");
    // (4) connecting to slave
    error_code = client.connect(slave_address);
    if(error_code)
    {
        std::printf("failed to conect to server. \n");
        std::cout<<"error: "<<error_code.message()<<"\n";
        return 0;
    }
    else
    {
        client.getServerData(slave_address,servers[1]);
        print_metadata(servers[1]);
    }
    /*
    // (5) send new firmware to master chip
    std::printf("*---------------------------------------------*\n");
    std::printf("trying to update firmware on the master chip...\n");
    std::string master_firmware = argv[2];
    error_code = client.uploadApp(master_address,master_firmware);
    if(error_code)
    {
        std::printf("failed to upload firmware. \n");
        std::cout<<"error: "<<error_code.message()<<"\n";
        return 0;
    }
    else
    {
        std::cout<<"new firmware uploaded.\n";
    }
    
    // (6) send new firmware to slave chip
    std::printf("*---------------------------------------------*\n");
    std::printf("trying to update firmware on the slave chip...\n");
    std::string slave_firmware = argv[3];
    error_code = client.uploadApp(slave_address,slave_firmware);
    if(error_code)
    {
        std::printf("failed to upload firmware. \n");
        std::cout<<"error: "<<error_code.message()<<"\n";
    }
    else
    {
        std::cout<<"new firmware uploaded.\n";
    }
    
    std::printf("*---------------------------------------------*\n");
    std::printf("success, program exit...\n");
    */
    return 0;
}

static void print_metadata(sm::ServerData& server_data)
{
    std::printf("*---------------------------------------------*\n");
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
    std::printf("*---------------------------------------------*\n");
}
