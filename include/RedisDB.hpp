#ifndef REDIS_DB_HPP
#define REDIS_DB_HPP

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include<chrono>
// This will follow the singleton design pattern
class RedisDB
{
public:
    static RedisDB& getInstance(); //get the singlton instance

     //if successful, return true
    [[nodiscard]] bool dump(const std::string& fileName);
    
    //if successful, return true
    [[nodiscard]] bool load(const std::string& filename);

    void flushAll();
    
    //Key/value
    void set(const std::string& key, const std::string& value);
    
    [[nodiscard]] bool get(const std::string& key, std::string& value);

    std::vector<std::string> keys();
    
    std::string type(const std::string& key);
    
    [[nodiscard]]bool del(const std::string& key);
    
    [[nodiscard]]bool expire(const std::string& key, int seconds);
    
    [[nodiscard]]bool rename(const std::string& oldKey, const std::string& newKey);
private:
    RedisDB()  = default;
    ~RedisDB() = default;

    RedisDB(const RedisDB&)             = delete;
    RedisDB& operator=(const RedisDB&)  = delete;




private:
    std::mutex m_dbMutex;
    
    std::unordered_map<std::string ,std::string> m_kvStore;
    std::unordered_map<std::string, std::vector<std::string>> m_listStore; 
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_hashStore;
    std::unordered_map<std::string,std::chrono::steady_clock::time_point> m_expiryMap;
};

#endif
