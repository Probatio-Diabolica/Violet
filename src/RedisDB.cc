#include "../include/RedisDB.hpp"

#include <sstream>
#include <fstream>
#include <ios>
#include <mutex>
#include <string>
#include <unordered_map>

RedisDB& RedisDB::getInstance()
{
    static RedisDB instance;
    return instance;
}








bool RedisDB::dump(const std::string& fileName)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    std::ofstream ofs(fileName,std::ios::binary); //object that will write to a file in finary format
    if(!ofs) return false;

    for(const auto& [key,value]: m_kvStore)
    {
        ofs<<'K' << key << ' ' << value << '\n';
    }

    for(const auto& [key,value] :  m_listStore)
    {
        ofs << 'L' << key;
        for(const auto& item : value )
        {
            ofs << ' ' << item;
        }
        ofs << '\n';
    }

    for(const auto& [key,value] : m_hashStore)
    {
        ofs << 'H' <<key;
        for(const auto& [subKey,subValue] : value)
        {
            ofs << ' '<< subKey << ':' <<subValue;
        }
        ofs <<'\n'; 
    }
    return true;
}


/*

K = Key - value

L = List

H = hash

*/ 
bool RedisDB::load(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    std::ifstream ifs(filename,std::ios::binary); //object that will read from a binary
    if(!ifs) return false;

    m_kvStore.clear();
    m_listStore.clear();
    m_hashStore.clear();

    std::string line;

    while(std::getline(ifs,line))
    {
        std::istringstream iss(line);
        char type;
        iss >> type ;
        if(type == 'K')
        {
            std::string key,value;
            iss >> key >> value;
            m_kvStore[key] = value;
        }
        else if(type == 'L')
        {
            std::string key;
            iss >> key;
            std::string item;
            std::vector<std::string> list;
            while(iss >> item)
            {
                list.push_back(item);
            }
            m_listStore[key]=list;
        }
        else if(type=='H')
        {
            std::string key;
            iss >> key;
            std::unordered_map<std::string, std::string> hash;
            std::string pair;
            while(iss >> pair)
            {
                auto pos = pair.find(':');
                if(pos != std::string::npos)
                {
                    std::string field = pair.substr(0,pos);
                    std::string value = pair.substr(pos+1);
                    hash[field] = value;
                }
            }
            m_hashStore[key] = hash;
        }
    }

    return true;
}