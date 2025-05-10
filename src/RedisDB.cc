#include "../include/RedisDB.hpp"

#include <fstream>
#include <ios>
#include <mutex>

RedisDB& RedisDB::getInstance()
{
    static RedisDB instance;
    return instance;
}


bool RedisDB::dump(const std::string& fileName)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ofstream ofs(fileName,std::ios::binary);
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

bool RedisDB::load(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ifstream ifs(filename,std::ios::binary);
    if(!ifs) return false;

    m_kvStore.clear();
    m_listStore.clear();
    m_hashStore.clear();

    std::string line;

    return true;
}