#include"../include/RESPCommandHandler.hpp"

#include <cstdio>
#include <exception>
#include <iostream>
#include <algorithm>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper functions

//handling status
static std::string status(std::vector<std::string>& tokens, VioletDB& db)
{
   return "+service: [online]\r\n"; 
}


//handling echo
static std::string echo(std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error: expected a message to echo\n Hint: echo \"hello world\"\r\n";

    return '+' + tokens[1] + "\r\n";
}

//handle ping
static std::string flush(std::vector<std::string>& tokens,VioletDB& db)
{
    db.flushAll();
    return "+flushed\r\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// key value helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//set the key value pair
static std::string set(std::vector<std::string>& tokens,VioletDB& db)
{
    if(tokens.size() < 3) return "-error: SET command requires <key> and <value>\r\nUsage: SET mykey myvalue\r\n";
    else
    {
        db.set(tokens[1],tokens[2]);
        return "+done\r\n";
    }
}

//get the key-value pair
static std::string get(std::vector<std::string>& tokens,VioletDB& db)
{
    if(tokens.size() < 2) return "-error: GET command requires <key>\r\nUsage: GET mykey\r\n";

    else
    {
        std::string value;
        if(db.get(tokens[2],value)) return '$' + value.length() + "\r\n" + value + "\r\n";
        else return "$-1\r\n";
        
    }
}


static std::string keys(std::vector<std::string>& tokens,VioletDB& db)
{
    std::vector<std::string> altKeys = db.keys();
    std::string newKey = '*' + std::to_string(altKeys.size()) + "\r\n";
    for(const std::string& key : altKeys)
    {
        newKey +=  '$' + std::to_string(key.size()) + "\r\n" + key + "\r\n";
    }

    return newKey;
}


static std::string type(std::vector<std::string>& tokens,VioletDB& db)
{
    if(tokens.size() < 2) return "-error: Type requires key\r\n";
    else return '+' + db.type(tokens[1])+"\r\n";
}

static std::string del(std::vector<std::string>& tokens,VioletDB& db)
{
    bool res = false;
    if(tokens.size() < 2) return "-error: missing key \r\n";
    else res = db.del(tokens[1]);
    return ':' + std::to_string(res)+"\r\n";
}


static std::string expire(std::vector<std::string>& tokens,VioletDB& db)
{
    if(tokens.size() < 3) return  "-error: missing <key> <seconds>\nusage=> expire <key> <seconds>\r\n"; 
    
    int seconds = std::stoi(tokens[2]);

    return (db.expire(tokens[1],seconds) ? "+done\r\n" : "\r\n");  
}


static std::string rename(std::vector<std::string>& tokens,VioletDB& db)
{
    if(tokens.size() < 3) return "-error: => missing <old key> <new key>\nusage => rename <old key> <new key>\r\n";
    
    if(db.rename(tokens[1],tokens[2])) return "+done\r\n";
    return "\r\n";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//List operations

//fetches list elements at the given key and encodes them as a RESP array.
static std::string listGetAll(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error : missing <key>\r\n";

    const auto& key = tokens[1];
    const auto elements = db.lget(key);

    std::ostringstream oss;
    oss << '*' << elements.size() << "\r\n";

    for(const auto& el : elements )
    {
        oss << '$' << el.size() << "\r\n" << el << "\r\n";
    }

    return oss.str();
}

//returns the length of the list at a given key as a RESP integer
static std::string listLength(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error: missing <key>\r\n";
    
    const auto& key = tokens[1];
    const ssize_t len = db.llen(key);

    return ':' + std::to_string(len) + "\r\n";
}


//appends one or more values to a list, then returns the lenght in RESP integer
static std::string listPushFront(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error: missing <key> <value>\r\n";

    if(tokens.size() < 3) return "-error: missing <value>\r\n"; 
    


    const auto& key = tokens[1];
    for(size_t i = 2; i<tokens.size();++i)
    {
        db.lpush(key, tokens[i]);
    }

    return ':' + std::to_string(db.llen(key));
}

//appends one or more values to a list, then returns the length in RESP integer 
static std::string listPushBack(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error: missing <key> <value>\r\n";
    
    if(tokens.size() < 3) return "-error: missing <value>\r\n";

    const auto& key = tokens[1];

    for(size_t i =2 ; i < tokens.size(); ++i)
    {
        db.lpush(key, tokens[i]);
    }

    return ':' + std::to_string(db.llen(key)) + "\r\n";
}

//pops and returns the first element of the list in RESP bulk format. returns -1 if key not found.  
static std::string listPopFront(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error: missing <key>\r\n";

    std::string val;

    if(db.lpop(tokens[1], val)) return '$' + std::to_string(val.size()) + "\r\n" + val + "\r\n";

    return "$-1\r\n";
}

//pops and returns the last element of the list. returns -1 if not found
static std::string listPopBack(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error: missing <key>\r\n";

    std::string val;

    if(db.rpop(tokens[1],val)) return '$' + std::to_string(val.size()) + "\r\n" + val + "\r\n";

    return "$-1\r\n";
}

//removes all occurances of a value, then returns the count of number of elements removed in RESP integer 
static std::string listRemoveOccurrences(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size()) return "-error: missing <key> <count> <value>\r\n";

    try
    {
        int count = std::stoi(tokens[2]);
        int removed = db.lrem(tokens[1],count,tokens[3]);
        return ':' + std::to_string(removed) + "\r\n";
    }
    catch (std::exception&)
    {
        return "error: inavlid <count>\r\n";
    }
}

//it returns an element at given index from list
static std::string listGetAtIndex(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 3) return "-error: missing <key> <index>\r\n";

    try 
    {
        int index  = std::stoi(tokens[2]);
        std::string val;

        if(db.lindex(tokens[1],index,val)) return '$' + std::to_string(val.size()) + "\r\n" + val + "\r\n";
        
        return "$-1\r\n"; //index out of bound
    } 
    catch (std::exception& ) 
    {
        return "-error: invalid <index>\r\n";
    }
}

//sets the list at a particular index.
static std::string listSetAtIndex(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 4) return "-error: missing <key> <index> <value>\r\n";
    
    try {
        int idx = std::stoi(tokens[2]);
        const std::string & key = tokens[1];
        const std::string& value = tokens[3];
        if(db.lset(key, idx, value)) return ":done\r\n";
    } 
    catch (const std::exception& ) 
    {
        return "error: invalid <index>\r\n";    
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///Hash operations

//sets the hash field to the specified value. 
static std::string processHashSet(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 4) return "-error: missing <key> <field> <value>\r\n";
    
    db.hset(tokens[1],tokens[2] ,tokens[3]);

    return ":1\r\n";
}

//Gets the value of the hash field and then, if found, returns it as bulk string else returns $-1   
static std::string processHashGet(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 3) return "-error: missing\r\n";

    std::string value;
    
    if(db.hget(tokens[1], tokens[2], value)) return '$' + std::to_string(value.size()) + "\r\n" + value + "\r\n";
 
    return "$-1\r\n"; 
}

//checks existence, :1 if exists, else :0 
static std::string processHashExists(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 3) return "-error: missing <key> <field>\r\n";

    return ':' + std::to_string(db.hexists(tokens[1],tokens[2])) + "\r\n";
}

//deleting a field from the hash, :1 if removed else :0
static std::string processHashDeleteField(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 3) return "-error: missing <key> <field>\r\n";
    
    return ':' + std::to_string(db.hdel(tokens[1],tokens[2])) + "\r\n";
}

//returns all fields and the values from the hash store
static std::string processHashGetAll(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error: missing <key>\r\n";
    
    const auto& hash = db.hgetall(tokens[1]);
    std::ostringstream oss;
    oss << '*' <<hash.size()*2 << "\r\n";
    for(const auto& [field,value] : hash)
    {
        oss << '$' << field.size() << "\r\n" << field << "\r\n";
        oss << '$' << value.size() << "\r\n" << value << "\r\n";
    }

    return oss.str();
}

//retuns the RESP array of keys
static std::string processHashKeys(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error: missing <key>\r\n";

    const auto& fields = db.hkeys(tokens[1]);
    std::ostringstream oss;
    oss << '*' << fields.size() << "\r\n";
    for(const auto& field: fields)
    {
        oss << '$' << field.size() << "\r\n" << field << "\r\n";
    }

    return oss.str();
}

//getting the resp array of values
static std::string processHashValues(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error:  missing <key>\r\n";

    const auto& values = db.hvals(tokens[1]);
    std::ostringstream oss;
    for(const auto& val : values)
    {
        oss << '$' << val.size() << "\r\n" << val << "\r\n";
    }

    return oss.str();
}

//return the number of fields in RESP integer
static std::string processHashLength(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 2) return "-error: missing <keys>\r\n";

    ssize_t len = db.hlen(tokens[1]);
    return ':' + std::to_string(len) + "\r\n";
}

static std::string processHashMultiSet(const std::vector<std::string>& tokens, VioletDB& db)
{
    if(tokens.size() < 4 or (tokens.size()%2)==1) return "-error: expected <key> followed by <field> <value> pairs\r\n";
    
    std::vector<std::pair<std::string,std::string>> fieldValues;

    for(size_t i = 2; i < tokens.size();i+=2)
    {
        fieldValues.emplace_back(tokens[i],tokens[i+1]);
    }
    db.hmset(tokens[1], fieldValues);
    return ":done\r\n";
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//non static help function

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///class definitions


RESPCommandHandler::RESPCommandHandler()
{

}



std::string RESPCommandHandler::processCommand(const command& cmdLine)
{
    //get the tokens
    std::vector<std::string> tokens = getRespTokens(cmdLine);

    if(tokens.empty()) return "-error: empty command\r\n";

    command cmd = tokens[0];
        
    std::transform(cmd.begin(),cmd.end(),cmd.begin(),::toupper);
    std::ostringstream response;

    //Connecting to the database
    VioletDB& db = VioletDB::getInstance();



    //Checking commands
    if(cmd=="STATUS")  return status(tokens,db);
    
    else if(cmd == "ECHO") return echo(tokens, db);
    
    else if(cmd == "FLUSHALL") return flush(tokens, db);
    

    //key - value ops
    else if(cmd == "SET") return set(tokens,db);    
    
    else if(cmd == "GET") return get(tokens,db);
    
    else if(cmd == "KEYS") return keys(tokens,db);
    
    else if(cmd == "TYPE") return type(tokens,db);
    
    else if(cmd =="DEL" || cmd =="UNLINK") return del(tokens,db);
    
    else if(cmd=="EXPIRE") return expire(tokens,db);

    else if(cmd=="RENAME") return rename(tokens,db);
    
    //List ops
    else if (cmd == "LGET") return listGetAll(tokens, db);
    
    else if (cmd == "LLEN") return listLength(tokens, db);
    
    else if (cmd == "LPUSH") return listPushFront(tokens, db);
    
    else if (cmd == "RPUSH") return listPushBack(tokens, db);
    
    else if (cmd == "LPOP") return listPopFront(tokens, db);
    
    else if (cmd == "RPOP") return listPopBack(tokens, db);
    
    else if (cmd == "LREM") return listRemoveOccurrences(tokens, db);
    
    else if (cmd == "LINDEX") return listGetAtIndex(tokens, db);
    
    else if (cmd == "LSET") return listSetAtIndex(tokens, db);

    
    //Hash ops
    else if (cmd == "HSET") return processHashSet(tokens, db);
    
    else if (cmd == "HGET") return processHashGet(tokens, db);
    
    else if (cmd == "HEXISTS") return processHashExists(tokens, db);
    
    else if (cmd == "HDEL") return processHashDeleteField(tokens, db);
    
    else if (cmd == "HGETALL") return processHashGetAll(tokens, db);
    
    else if (cmd == "HKEYS") return processHashKeys(tokens, db);
    
    else if (cmd == "HVALS") return processHashValues(tokens, db);
    
    else if (cmd == "HLEN") return processHashLength(tokens, db);
    
    else if (cmd == "HMSET") return processHashMultiSet(tokens, db);

    else response<< "unknown command \r\n";

    return response.str();
}
