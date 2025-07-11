#include "../include/Server.hpp"
#include "../include/RESPCommandHandler.hpp"
#include "../include/VioletDB.hpp"

#include <asm-generic/socket.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<signal.h>
#include<vector>

static Server* g_server = nullptr;

static void signalHandler(int signum)
{
    if(g_server)
    {
        std::cout<<" >> Caught the signal ["<<signum<<"]\nShutting down...\n";
        g_server->shutdown();
    }

    exit(signum);
}


void Server::setupSignalHandler()
{
    signal(SIGINT, signalHandler);
}


Server::Server(int port) 
    : m_port(port),m_sockfd(-1),m_running(true)
{
    g_server = this;
    setupSignalHandler();
}

void Server::run()
{
    m_sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(m_sockfd < 0)
    {
        std::cerr<<"Ran out of network descriptors\n";
        return;
    }

    int opt = 1;

    setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR,& opt , sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(m_port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;


    if(bind(m_sockfd, (struct sockaddr*)& serverAddr, sizeof(serverAddr))<0)
    {
        std::cerr<<"Error Binding Server FD \n";
        return;
    }


    if(listen(m_sockfd,10)<0)
    {
        std::cerr<<"Error listening on server FD\n";
        return;
    }

    std::cout<<"Serter started listening on port: " << m_port <<'\n';

    std::vector<std::thread> clientThreads;
    RESPCommandHandler cmdHandler;

    while(m_running)
    {
        int clientFd = accept(m_sockfd,nullptr ,nullptr  );
        if(clientFd < 0) //the socket is not connect :(
        {
            if(m_running) std::cerr<<"Error accepting client connection\n";
            break;
        }
        clientThreads.emplace_back([clientFd,&cmdHandler]()
        {
            //lambda for thread

            char buffer[1024]; //buffersize 1 KB
            while(true)
            {
                // std::fill(buffer.begin(),buffer.end(),0);
                memset(buffer, 0, sizeof(buffer));
                int bytes = recv(clientFd, buffer, sizeof(buffer)-1,0);
                if(bytes <= 0) break;
                std::string request(buffer,bytes);
                
                std::string response = cmdHandler.processCommand(request);
                
                send(clientFd,response.c_str(),response.size(),0);
            }
            close(clientFd);
        });
    }

    for(auto& t: clientThreads)
    {
        if(t.joinable())  t.join();
    }

    //save the data before shutting down
    if(VioletDB::getInstance().dump("dump.dbz")) std::cout<<"DB dumped to dump.dbz\n";
    else std::cerr<<"Error dumping\n";
}


void Server::shutdown()
{
    m_running = false;

    if(m_sockfd==-1)
    {
        if(VioletDB::getInstance().load("dump.dbz"))
            std::cout << "Database loaded from dump.dbz\n";
        else 
            std::cout << "Cannot dump to dump.dbz\n";
   
        close(m_sockfd);
    }

    std::cout<<"Server shutdown gracefully\n";
}


Server::~Server()
{

}
