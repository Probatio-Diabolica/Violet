#include "../include/Server.hpp"
#include "../include/VioletDB.hpp"

#include<filesystem>
#include<cstdlib>
#include<iostream>
#include<string>
#include<thread>
#include<chrono>

namespace fs = std::filesystem;



int main(int argc, char* argv[])
{
    int port=2005;
    if(argc >= 2) port = std::stoi(argv[1]);

    const char* dumpPathEnv = std::getenv("DUMP_PATH");
    std::string dumpPath = dumpPathEnv ? dumpPathEnv : "data/dump.dbz";

    fs::create_directories(fs::path(dumpPath).parent_path());
    
    if(VioletDB::getInstance().load(dumpPath))
        std::cout << "Database loaded from "<< dumpPath<<'\n';
    else  std::cout << "No dump found\n";

    Server server(port);
    std::cout<<"Chosen port was "<<port<<"\n";

    // dump the database every 30 seconds for demonstration, else the orignal plan is to make it dump every 150 seconds
    std::thread pThread([&]()
    {
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(30));
            //!! dump the database here
            if(VioletDB::getInstance().dump(dumpPath)) std::cout<<"Database Dumped\n";
            else std::cerr<<"Error dumping the database\n";
        }
    });

    pThread.detach();
        
    server.run();
}
