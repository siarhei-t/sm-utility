/**
 * @file sm_client.cpp
 *
 * @brief 
 *
 * @author Siarhei Tatarchanka
 *
 */

#include <algorithm>
#include <iostream>
#include "../inc/sm_client.hpp"
#include "sm_client.hpp"

namespace sm
{
    Client::Client()
    {
        client_thread = std::make_unique<std::thread>(std::thread(&Client::clientThread,this));
    }

    Client::~Client()
    {
        thread_stop.store(true, std::memory_order_relaxed);
        client_thread->join();
    }

    void Client::connect(const std::uint8_t address)
    {
        if(server_id == not_connected)
        {
            //create server and select server first
            addServer(address);
        }
        //ping server
        //if ping success -> load info
    }

    void Client::disconnect()
    {
        if(server_id != not_connected)
        {
            servers[server_id].info.status = ServerStatus::Unavailable;
            server_id = not_connected;
        }
    }

    void Client::addServer(const std::uint8_t address)
    {
        auto it = std::find_if(servers.begin(),servers.end(),[]( ServerData& server){return server.info.status == ServerStatus::Unavailable;} );
        if(it != servers.end())
        {
            int index =  std::distance(servers.begin(), it);
            servers[index] = ServerData();
            servers[index].info.addr = address;
            server_id = index;
        }
        else
        {
            servers.push_back(ServerData());
            server_id = servers.size() - 1;
        }
    }

    void Client::clientThread()
    {
        using namespace std::chrono_literals;
        while (!thread_stop.load(std::memory_order_relaxed))
        {
            std::cout<<"test output..."<<"\n";
            std::this_thread::sleep_for(500ms);
        }
    }
    void Client::callServerExchange(const size_t resp_length)
    {
        
    }
}
