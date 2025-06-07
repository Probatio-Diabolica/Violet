#ifndef SERVER_HPP
#define SERVER_HPP
#include <atomic>


class Server
{
public:
    Server(int port);
    
    void run();
    
    void shutdown();
    
    ~Server();

private:
    int m_port;
    int m_sockfd;
    std::atomic<bool> m_running;

    //sets up  signal handling for  graceful shutdown
    void setupSignalHandler();
};



#endif
