#ifndef REDIS_DB_HPP
#define REDIS_DB_HPP

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// This will follow the singleton design pattern
class RedisDB
{
public:
    static RedisDB& getInstance(); //get the singlton instance

     //if successful, return true
    [[nodiscard]] bool dump(const std::string& fileName);
    
    //if successful, return true
    [[nodiscard]] bool load(const std::string& filename);

private:
    RedisDB()  = default;
    ~RedisDB() = default;

    RedisDB(const RedisDB&)             = delete;
    RedisDB& operator=(const RedisDB&)  = delete;

    std::mutex db_mutex;
    
    std::unordered_map<std::string ,std::string> m_kvStore;
    std::unordered_map<std::string, std::vector<std::string>> m_listStore; 
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_hashStore;

};

#endif