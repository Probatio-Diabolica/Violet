#include "../include/VioletDB.hpp"
#include<iostream>
#include <chrono>
#include <iterator>
#include <sstream>
#include <fstream>
#include <ios>
#include <vector>
#include <algorithm>


VioletDB& VioletDB::getInstance()
{
    static VioletDB instance;
    return instance;
}




void VioletDB::flushAll()
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    m_kvStore.clear();
    m_listStore.clear();
    m_hashStore.clear();
}

void VioletDB::set(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    m_kvStore[key]= value;
}

bool VioletDB::get(const std::string& key, std::string& value)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    evictExpiredKeys();
    
    if(const auto it = m_kvStore.find(key); it != m_kvStore.end())
    {
        value = it ->second;
        return true;
    }

    return false;
}



std::vector<std::string> VioletDB::keys()
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    evictExpiredKeys();

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

void VioletDB::evictExpiredKeys()
{
    auto now = std::chrono::steady_clock::now();

    std::erase_if(m_expiryMap,[&](const auto& expired)
    {
        const auto& [expiredKey,time] = expired;
        if(now > time)
        {
            m_kvStore.erase(expiredKey);
            m_listStore.erase(expiredKey);
            m_hashStore.erase(expiredKey);
            return true;
        }
        return false;
    });
}

    
std::string VioletDB::type(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    evictExpiredKeys();
    
    if(m_kvStore.find(key) != m_kvStore.end())
        return "string";
    

    if(m_listStore.find(key) != m_listStore.end())
        return "list";
    

    if(m_hashStore.find(key) != m_hashStore.end())
        return "hash";
    
    return "none";
}
    
bool VioletDB::del(const std::string& key)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    evictExpiredKeys();

    bool erased = false;
    erased |= m_kvStore.erase(key) > 0 ;
    erased |= m_hashStore.erase(key) > 0 ;
    erased |= m_listStore.erase(key) > 0 ;
    return erased;

}
    
bool VioletDB::expire(const std::string& key, int seconds)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    evictExpiredKeys();

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
    
bool VioletDB::rename(const std::string& oldKey, const std::string& newKey)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    evictExpiredKeys();
    
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

bool VioletDB::dump(const std::string& fileName)
{
    std::lock_guard<std::mutex> lock(m_dbMutex);
    std::ofstream ofs(fileName,std::ios::binary); //object that will write to a file in finary format
    if(!ofs) return false;

    for(const auto& [key,value]: m_kvStore)
    {
        ofs<<"K " << key << ' ' << value << '\n';
    }

    for(const auto& [key,value] :  m_listStore)
    {
        ofs << "L " << key;
        for(const auto& item : value )
        {
            ofs << ' ' << item;
        }
        ofs << '\n';
    }

    for(const auto& [key,value] : m_hashStore)
    {
        ofs << "H " <<key;
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
bool VioletDB::load(const std::string& filename)
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

//list ops
     
std::vector<std::string> VioletDB::lget(const std::string& key)
{
    std::lock_guard lock(m_dbMutex);
    auto it = m_listStore.find(key);
    return (it!= m_listStore.end()) ? it->second : std::vector<std::string>{};
}
    

ssize_t VioletDB::llen(const std::string& key)
{
    std::lock_guard lock(m_dbMutex);

    auto it = m_listStore.find(key);
    return (it!=m_listStore.end()) ? static_cast<ssize_t>(it->second.size()) : 0;
}
    

void VioletDB::lpush(const std::string& key, const std::string& value)
{
    std::lock_guard lock(m_dbMutex);
    m_listStore[key].insert(m_listStore[key].begin(),value); 
}


void VioletDB::rpush(const std::string& key, const std::string& value)
{
    std::lock_guard lock(m_dbMutex);
    m_listStore[key].push_back(value);
}


bool VioletDB::lpop(const std::string& key, std::string& value)
{
    std::lock_guard lock(m_dbMutex);
    auto it = m_listStore.find(key);


    if(it!= m_listStore.end() and !it->second.empty())
    {
        value = std::move(it->second.front());
        it->second.erase(it->second.begin());
        return true;
    }

    return false;
}


bool VioletDB::rpop(const std::string& key, std::string& value)
{
    std::lock_guard lock(m_dbMutex);
    auto it = m_listStore.find(key);

    if(it != m_listStore.end() and !it->second.empty())
    {
        value = std::move(it->second.back());
        it->second.pop_back();
        return true;
    }

    return false;
}


int VioletDB::lrem(const std::string& key, int count, const std::string& value)
{
    std::lock_guard lock(m_dbMutex);
    auto it = m_listStore.find(key);
    if(it == m_listStore.end()) return 0;

    auto& list = it->second;
    int removed = 0;

    if(count == 0)
    {
        auto newEnd = std::remove(list.begin(),list.end(),value);
        removed = std::distance(newEnd, list.end());
        list.erase(newEnd,list.end());
    }
    else if(count > 0)
    {
        for(auto it = list.begin(); it != list.end() and removed < count;)
        {
            if(*it == value)
            {
                it = list.erase(it);
                ++removed;
            }
            else ++it;
        }
    }
    else 
    {
        //if count == 0, remove from behind
        for(auto it = list.rbegin();it != list.rend() and removed < -count;)
        {
            if(*it == value)
            {
                //converting the r_it to normal it
                auto baseIt = std::next(it).base();
                it = std::make_reverse_iterator(list.erase(baseIt));
                ++removed;
            }
            else ++it;
        }
    }

    return removed;
}


bool VioletDB::lindex(const std::string& key, int index, std::string& value)
{
    std::lock_guard lock(m_dbMutex);

    auto it  = m_listStore.find(key);
    if(it == m_listStore.end()) return false;
    

    const auto& list = it->second;
    if(index < 0) index += list.size();
    if(index < 0 or index >= static_cast<int>(list.size())) return false;

    value = list[index];
    return true;
}


bool VioletDB::lset(const std::string& key, int index, const std::string& value)
{
    std::lock_guard lock(m_dbMutex);

    auto it = m_listStore.find(key);
    if(it == m_listStore.end()) return false;

    auto& list = it->second;
    if(index < 0 ) index += list.size();
    if(index < 0 or index >= static_cast<int>(list.size())) return false;

    list[index] = value;
    return true;
}


//Hash ops
bool VioletDB::hset(const std::string& key, const std::string& field, const std::string& value)
{
    std::lock_guard lock(m_dbMutex);
    m_hashStore[key][field] = value;

    return true;
}

bool VioletDB::hget(const std::string& key, const std::string& field, std::string& value)
{
    std::lock_guard lock(m_dbMutex);
    if(auto it = m_hashStore.find(key);it != m_hashStore.end())
    {
        if(auto fit = it->second.find(field); fit != it->second.end())
        {
            value = fit->second;
            return true;
        }
    }

    return false;
}

bool VioletDB::hexists(const std::string& key, const std::string& field)
{
    std::lock_guard lock(m_dbMutex);
    if(auto it = m_hashStore.find(key); it != m_hashStore.end())
    {
        return it->second.contains(field);
    }

    return false;
}


bool VioletDB::hdel(const std::string& key, const std::string& field)
{
    std::lock_guard lock(m_dbMutex);
    if(auto it = m_hashStore.find(key); it != m_hashStore.end())
    {
        return it->second.erase(field) > 0;
    }

    return false;
}


std::unordered_map<std::string, std::string> VioletDB::hgetall(const std::string& key)
{
    std::lock_guard lock(m_dbMutex);

    if(auto it = m_hashStore.find(key); it != m_hashStore.end())
    {
        return it->second;
    }

    return {};
}

std::vector<std::string> VioletDB::hkeys(const std::string& key)
{
    std::lock_guard lock(m_dbMutex);
    std::vector<std::string> fields;
    
    if(auto it = m_hashStore.find(key); it != m_hashStore.end())
    {
        fields.reserve(it->second.size());

        for(const auto& [k, val] : it->second)
        {
            fields.push_back(val);
        }
    }

    return fields;
}    

std::vector<std::string> VioletDB::hvals(const std::string& key)
{
    std::lock_guard lock(m_dbMutex);
    std::vector<std::string> values;
    
    if(auto it = m_hashStore.find(key); it != m_hashStore.end())
    {
        values.reserve(it->second.size());
        for(const auto& [k,val]: it->second)
        {
            values.push_back(val);
        }
    }
    return values;
}

ssize_t VioletDB::hlen(const std::string& key)
{
    std::lock_guard lock(m_dbMutex);

    if(auto it = m_hashStore.find(key); it != m_hashStore.end())
    {
        return static_cast<ssize_t>(it->second.size());
    }
    
    return 0;
}

bool VioletDB::hmset(const std::string& key, const std::vector<std::pair<std::string, std::string>>& fieldValues)
{
    std::lock_guard lock(m_dbMutex);

    auto& fields = m_hashStore[key];

    for(const auto& [field,val] : fieldValues)
    {
        fields[field] = val;
    }

    return true;
}

