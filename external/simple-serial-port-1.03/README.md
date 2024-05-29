# simple-serial-port

[![CMake](https://github.com/gbmhunter/CppLinuxSerial/actions/workflows/cmake.yml/badge.svg)](https://github.com/gbmhunter/CppLinuxSerial/actions/workflows/cmake.yml)

A small library written in modern C++ for working with a serial port with limited configuration options, designed to work with chips such as ft232, ch340, etc. Can be built for Windows and Linux.

## Table of contents
* [Features](#features)
* [Building](#building)
* [Examples](#examples)
* [License](#license)

## Features

Supported baudrates:  9600, 19200, 38400, 57600, 115200

## Building

In the example, configuration is done using Cmake. Ninja is used as the default build tool. However, any other build tool can also be used.

**Configure for Windows:** 

```sh
cmake -DTARGET_WINDOWS=ON -Bbuild
```
**Configure for Linux:** 

```sh
cmake -DTARGET_LINUX=ON -Bbuild
```

**Building:**
```sh
cmake --build build
```

## Examples

**SerialPort:**

```c++
#include <iostream>
#include "serial_port.hpp"

int main()
{
    //create serial port instance
    sp::SerialPort serial_port;
    //test string to send and read
    std::string test = "Hello world!";
    //test buffer to read data from port
    std::vector<std::uint8_t> buffer;
    //open serial port
    auto error = serial_port.open("COM1");
    if(error.value() == 0)
    {
        //setup port in case of no errors
        //default config : 9600 baudrate, 8 databits, 1 stop bit, no parity, 1s timeout
        error = serial_port.setup(sp::PortConfig()); 
        if(error.value() == 0)
        {
            //write data to port   
            serial_port.port.writeString(test);
            //reading data from port, read size used the same as test string has, 
            //because we are trying to read the same data in case of TX/RX pin shorting
            auto bytes_read = serial_port.port.readBinary(buffer,test.size());
            //create string from raw received data 
            std::string responce(buffer.begin(),buffer.end());
            std::cout<<"read from port :"<<responce<<"\n";
            //close port
            serial_port.port.closePort();
        }
    }
    return 0;
}
```

**SerialDevice:**

```c++
#include <iostream>
#include "serial_port.hpp"

int main(void)
{
    //create instance of serial device
    sp::SerialDevice serial_device;
    //get list of available serial ports in the system
    auto actual_list = serial_device.getListOfAvailableDevices();
    if(actual_list.size() > 0)
    {
        //print available ports
        for(auto i = 0; i < actual_list.size(); ++i)
        {
            std::cout<<" available device : "<<actual_list[i]<<" | index : "<<i<<"\n";
        }
    }
    else
    {
        std::cout<<"no available devises found, exit..."<<"\n";
    }
    
    return 0;
}
```

## License

[MIT](https://choosealicense.com/licenses/mit/)
