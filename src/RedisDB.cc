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

    for(const auto& [key,value]: kv_store)
    {
        ofs<<'K' << key << ' ' << value << '\n';
    }

    for(const auto& [key,value] :  list_store)
    {
        ofs << 'L' << key;
        for(const auto& item : value )
        {
            ofs << ' ' << item;
        }
        ofs << '\n';
    }

    for(const auto& [key,value] : hash_store)
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

    kv_store.clear();
    list_store.clear();
    hash_store.clear();

    std::string line;

    return true;
}