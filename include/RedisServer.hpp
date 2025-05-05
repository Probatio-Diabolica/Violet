#ifndef REDIS_SERVER_HPP
#define REDIS_SERVER_HPP
#include <atomic>


class RedisServer
{
public:
    RedisServer(int port);
    void run();
    void shutdown();
    ~RedisServer();
private:
    int m_port;
    int m_sockfd;
    std::atomic<bool> m_running;
};

#endif