#include "../include/RedisDB.hpp"

#include <chrono>
#include <sstream>
#include <fstream>
#include <ios>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

RedisDB& RedisDB::getInstance()
{
    static RedisDB instance;
    return instance;
}




void RedisDB::flushAll()
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    m_kvStore.clear();
    m_listStore.clear();
    m_hashStore.clear();
}

void RedisDB::set(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    m_kvStore[key]= value;
}

bool RedisDB::get(const std::string& key, std::string& value)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    auto it  = m_kvStore.find(key);
    if(it!=m_kvStore.end())
    {
        value = it->second;
        return true;
    }
    return false;
}



std::vector<std::string> RedisDB::keys()
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    std::vector<std::string> out;
    
    //keys from key value store
    for(const auto& [key,value] : m_kvStore)
    {
        out.push_back(key);
    }

    //keys from hash store
    for(const auto& [key,value] : m_hashStore)
    {
        out.push_back(key);
    }
    //keys from list store 
    for(const auto& [key,value] : m_listStore)
    {
        out.push_back(key);
    }
    return out;
}



    
std::string RedisDB::type(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    
    if(m_kvStore.find(key) != m_kvStore.end())
        return "string";
    

    if(m_listStore.find(key) != m_listStore.end())
        return "list";
    

    if(m_hashStore.find(key) != m_hashStore.end())
        return "hash";
    
    return "none";
}
    
bool RedisDB::del(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    bool erased = false;
    erased |= m_kvStore.erase(key) > 0 ;
    erased |= m_hashStore.erase(key) > 0 ;
    erased |= m_listStore.erase(key) > 0 ;
    return false;

}
    
bool RedisDB::expire(const std::string& key, int seconds)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    bool exists = (m_kvStore.find(key)    != m_kvStore.end()) 
                  or (m_hashStore.find(key)  != m_hashStore.end()) 
                  or (m_listStore.find(key)  != m_listStore.end()) ;
    if(exists)
    {
        m_expiryMap[key] = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
        return true;
    }
    return false;
}
    
bool RedisDB::rename(const std::string& oldKey, const std::string& newKey)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    bool found = false;

    auto oldKV = m_kvStore.find(oldKey);
    if(oldKV != m_kvStore.end())
    {
        m_kvStore[newKey] = oldKV->second;
        m_kvStore.erase(oldKV);
        found = true;
    }
    
    auto oldHash = m_hashStore.find(oldKey);
    if(oldHash != m_hashStore.end())
    {
        m_hashStore[newKey] = oldHash->second;
        m_hashStore.erase(oldHash);
        found = true;
    }
    
   auto oldList = m_listStore.find(oldKey);
    if(oldList != m_listStore.end())
    {
        m_listStore[newKey] = oldList->second;
        m_listStore.erase(oldList);
        found = true;
    }

   auto oldExpire = m_expiryMap.find(oldKey);
    if(oldExpire != m_expiryMap.end())
    {
        m_expiryMap[newKey] = oldExpire->second;
        m_expiryMap.erase(oldExpire);
        found = true;
    }
 

    return found;
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
        else if(type == 'H')
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
