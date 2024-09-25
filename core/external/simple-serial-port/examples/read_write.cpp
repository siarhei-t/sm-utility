/**
 * @file read_write.hpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <iostream>
#include <string>
#include "serial_port.hpp"

int main(void)
{
    sp::SerialDevice sys_serial;
    auto actual_list = sys_serial.getListOfAvailableDevices();
    if(actual_list.size() > 0)
    {
        for(auto i = 0; i < actual_list.size(); ++i)
        {
            std::cout<<" available device : "<<actual_list[i]<<" | index : "<<i<<"\n";
        }
        std::string input;
        int index = -1;
        std::cout<<" please select port to test (index no.) : ";
        std::cin>>input;
        try
        {
            index = std::stoi(input);
        }
        catch (std::invalid_argument const& ex)
        {
            std::cout <<"invalid argumant passed, exit...\n";
            return 0;
        }
        if((index >= 0) &&  (index <= actual_list.size() - 1))
        {
            std::cout<<"selected index : "<<index<<"\n";
            sp::SerialPort test(actual_list[index],sp::PortConfig());
            if(test.getState() == sp::PortState::Open)
            {
                std::cout<<"port on path " <<test.getPath()<<" opened succesfully." <<"\n";
                std::string data_to_send = "This is a test string with length more than 32 bytes";
                std::vector<std::uint8_t> data_to_read;
                //in case of shorted TX/RX pins test string should return in read function
                std::cout<<"DATA SENT :"<<data_to_send<<"\n"<<"\n";
                test.port.writeString(data_to_send);
                auto bytes_read = test.port.readBinary(data_to_read,data_to_send.size());
                std::cout<<"BYTES READ :"<<bytes_read<<"\n";
                std::string received_data(data_to_read.begin(),data_to_read.end());
                std::cout<<"DATA READ :"<<received_data<<"\n"<<"\n";
                test.port.closePort();
                std::cout<<" test finished, exit..." <<"\n";
            }
            else
            {
                std::cout<<" failed with port opening, do you have read/write permisisons?" <<"\n";
            }
        }
        else
        {
            std::cout<<" wrong index passed, exit.." <<"\n";
        }
    }
    else
    {
        std::cout<<"no available devises found, exit..."<<"\n";
    }
    
    return 0;
}
