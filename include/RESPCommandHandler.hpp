#ifndef RESP_COMMAND_HANDLER_HPP
#define RESP_COMMAND_HANDLER_HPP

#include "VioletDB.hpp"
#include <functional>
#include <string>
#include <unordered_map>


//parses the RESP commands
class RESPCommandHandler
{
public:
    using command = std::string;


    RESPCommandHandler();


    std::string processCommand(const command& cmdLine );
private:
    
};

#endif
