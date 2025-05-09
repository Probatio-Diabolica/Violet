#include "../include/RedisServer.hpp"
#include "../include/RedisDB.hpp"


#include<iostream>
#include <string>
#include<thread>
#include<chrono>

int main(int argc, char* argv[])
{
    int port=2005;
    if(argc >= 2) port = std::stoi(argv[1]);
    RedisServer server(port);
    std::cout<<"Chosen port was "<<port<<"\n";

    // dump the database every 300 seconds
    std::thread pThread([]()
    {
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(300));
            //!! dump the database here
            if(RedisDB::getInstance().dump("dump.dbz")) std::cout<<"Database Dumped\n";
            else std::cerr<<"Error dumping the database\n";
        }
    });

    pThread.detach();

    server.run();
}