#include"../include/RedisCommandHandler.hpp"
#include "../include/RedisDB.hpp"

#include <iostream>
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

    // for(auto& t: tokens)
    // {
    //     std::cout<<t<<'\n';
    // }


    std::string cmd = tokens[0];
    std::transform(cmd.begin(),cmd.end(),cmd.begin(),::toupper);
    std::ostringstream response;

    //Connecting to the database
    RedisDB& db = RedisDB::getInstance();


    //Checking commands
    if(cmd=="PING") 
        response << "+pong\r\n";
    else if(cmd == "ECHO")
    {
        if(tokens.size() > 1) response<<"+"<<tokens[1]<<"\r\n";
    }
    else if(cmd == "FLUSHALL")
    {
        db.flushAll();
        response<<"+OK\r\n";
    } 
    else if(cmd == "SET") //key - value ops
    {
        if(tokens.size()<3) response<<"-Error: SET requires key and value\r\n";
        else{
            db.set(tokens[1],tokens[2]);
            response << "+OK\r\n";
        }
    }
    else if(cmd == "GET")
    {
        if(tokens.size() < 2)
        {
            response << "-Error: GET requires key and values\r\n";
        }else 
        {
            std::string value;
            if(db.get(tokens[1], value) ) response << "$" << value.size() << "\r\n" <<value << "\r\n";
            else response << "$-1\r\n";
        }
    }
    else if(cmd == "KEYS")
    {
        std::vector<std::string> altkeys = db.keys();
        response << "*" << altkeys.size() << "\r\n";
        for(const std::string& key: altkeys)
        {
            response << "$" << key.size() <<"\r\n" << key << "\r\n";
        }
    }
    else if(cmd == "TYPE")
    {
        if(tokens.size() < 2)
            response << "-Error: TYPE requires key\r\n";
        else 
            response << "+" << db.type(tokens[1])<<"\r\n";
        
    }
    else if(cmd =="DEL" || cmd =="UNLINK")
    {
        if(tokens.size() < 2)
            response << "-Error: " <<cmd <<" requires key \r\n";
        else
        {
            bool res = db.del(tokens[1]);
            response << ":" << (res ? 1 : 0) << "\r\n";
        }
    }
    else if(cmd=="EXPIRE")
    {
        if(tokens.size() < 3)
            response << "Expire requires key and time in seconds\r\n";
        else
        {
            int seconds = std::stoi(tokens[2]);
            if(db.expire(tokens[1],seconds))
                response << "+OK\r\n";
            else 
                response << "\r\n";
        }
    }
    else if(cmd=="RENAME")
    {
        if(tokens.size()  < 3)
            response << "-Error: RENAME missing key value names";
        else
        {
            if(db.rename(tokens[1],tokens[2]))
                response << "+OK\r\n";
            else
             response << "\r\n";
        }
    }
    else    
        response<< "error: unknown command \r\n";

    // todo : key value ops
    // todo : Hash ops
    // 

    return response.str();
}
