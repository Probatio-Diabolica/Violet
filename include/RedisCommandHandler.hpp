#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <string>


//parses the RESP commands
class RedisCommandHandler
{
public:
    using command = std::string;


    RedisCommandHandler();


    std::string processCommand(const command& cmdLine );
private:

};

#endif
