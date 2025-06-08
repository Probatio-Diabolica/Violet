#include "../include/Server.hpp"
#include "../include/VioletDB.hpp"


#include<iostream>
#include <string>
#include<thread>
#include<chrono>

int main(int argc, char* argv[])
{
    int port=2005;
    if(argc >= 2) port = std::stoi(argv[1]);
    
    if(VioletDB::getInstance().load("dump.dbz"))
        std::cout << "Database loaded from dump.dbz\n";
    else  std::cout << "No dump found\n";

    Server server(port);
    std::cout<<"Chosen port was "<<port<<"\n";

    // dump the database every 300 seconds
    std::thread pThread([]()
    {
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(30));
            //!! dump the database here
            if(VioletDB::getInstance().dump("dump.dbz")) std::cout<<"Database Dumped\n";
            else std::cerr<<"Error dumping the database\n";
        }
    });

    pThread.detach();
        
    server.run();
}
