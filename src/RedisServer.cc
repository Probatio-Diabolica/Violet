#include "../include/RedisServer.hpp"

#include <asm-generic/socket.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

static RedisServer* g_server = nullptr;


RedisServer::RedisServer(int port) 
    : m_port(port),m_sockfd(-1),m_running(true)
{
    g_server = this;
}

void RedisServer::run()
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

    std::cout<<"Serter started listening on port" << m_port <<'\n';
}


void RedisServer::shutdown()
{
    m_running = false;

    if(m_sockfd==-1) close(m_sockfd);

    std::cout<<"Server shutdown gracefully\n";
}


RedisServer::~RedisServer()
{

}