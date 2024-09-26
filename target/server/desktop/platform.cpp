/**
 * @file platform.cpp
 *
 * @brief 
 *
 * @author
 *
 */

#include "platform.hpp"
#include <cstdio>
#include <iostream>

std::string PlatformSupport::path;
sp::PortConfig PlatformSupport::config;

void DesktopTimer::platformStart()
{

}

void DesktopTimer::platformStop()
{

}

void DesktopCom::platformReadData(std::uint8_t data[], const size_t amount)
{
    (void)(data);
    (void)(amount);
    std::printf("port reading started...\n");
}

void DesktopCom::platformSendData(std::uint8_t data[], const size_t amount)
{
    (void)(data);
    (void)(amount);
}

void DesktopCom::platformFlush()
{

}

bool DesktopCom::platformInit()
{
    std::printf("platform init started...\n");
    std::string path = PlatformSupport::getPath();
    std::cout<<path<<"\n";
    return false;
}