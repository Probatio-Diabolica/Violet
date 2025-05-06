#include"../include/RedisCommandHandler.hpp"

#include<iostream>
#include <algorithm>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>


//parses the command and returns the tokenized vector
std::vector<std::string> getRespTokens(const std::string &command)
{
    std::vector<std::string> tokens;
    if(command.empty()) return tokens;
    if(command[0] != '*')
    {
        std::istringstream iss(command);
        std::string token;
        while(iss >> token)
        {
            tokens.push_back(token);
        }
    }

    size_t pos=0;

    if(command[pos]!='*') return tokens;
    ++pos;

    //crlf => carriage return  (\r) , Line Feed (\n)
    size_t crlf = command.find("\r\n",pos);
    if(crlf==std::string::npos) return tokens;

    int numElements = std::stoi(command.substr(pos,crlf-pos));
    pos = crlf +2;

    for(int i = 0 ; i < numElements ; i++)
    {
        //format error has occured
        if(pos >= command.size() or command[pos] != '$') break; 
        
        ++pos;
        crlf = command.find("\r\n",pos);
        if(crlf == std::string::npos) break;
        int len = std::stoi(command.substr(pos,crlf-pos));
        pos = crlf  + 2;

        if(pos +  len > command.size()) break;
        std::string token = command.substr(pos,len);
        tokens.push_back(token);
        pos += len +2; //skip the token and crlf
    }

    return tokens;
}


RedisCommandHandler::RedisCommandHandler()
{

}



std::string RedisCommandHandler::processCommand(const command& cmdLine)
{
    //get the tokens
    std::vector<std::string> tokens = getRespTokens(cmdLine);

    if(tokens.empty()) return "-Error: empty command\r\n";

    for(auto& t: tokens)
    {
        std::cout<<t<<'\n';
    }


    std::string cmd = tokens[0];
    std::transform(cmd.begin(),cmd.end(),cmd.begin(),::toupper);
    std::ostringstream response;

    //Connecting to the database

    //Checking commands

    return response.str();
}
