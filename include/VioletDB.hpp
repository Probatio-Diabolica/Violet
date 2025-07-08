#ifndef VIOLET_DB_HPP
#define VIOLET_DB_HPP

#include<mutex>
#include<string>
#include<unordered_map>
#include<vector>
#include<chrono>
// This will follow the singleton design pattern
class VioletDB
{
public:
    static VioletDB& getInstance(); //get the singlton instance

     //if successful, return true
    [[nodiscard]] bool dump(const std::string& fileName);
    
    //if successful, return true
    [[nodiscard]] bool load(const std::string& filename);

    void flushAll();
    
    //Key/value ops
    void set(const std::string& key, const std::string& value);
    
    [[nodiscard]] bool get(const std::string& key, std::string& value);

    std::vector<std::string> keys();
    
    std::string type(const std::string& key);
    
    [[nodiscard]]bool del(const std::string& key);
    
    [[nodiscard]]bool expire(const std::string& key, int seconds);
    
    [[nodiscard]]bool rename(const std::string& oldKey, const std::string& newKey);

    void evictExpiredKeys();

    //List ops
    std::vector<std::string> lget(const std::string& key);
    
    ssize_t llen(const std::string& key);
    
    void lpush(const std::string& key, const std::string& value);

    void rpush(const std::string& key, const std::string& value);

    bool lpop(const std::string& key, std::string& value);

    bool rpop(const std::string& key, std::string& value);
    
    int lrem(const std::string& key, int count, const std::string& value);
    
    bool lindex(const std::string& key, int index, std::string& value);
    
    bool lset(const std::string& key, int index, const std::string& value);
    
    //Hash ops
    bool hset(const std::string& key, const std::string& field, const std::string& value);

    bool hget(const std::string& key, const std::string& field, std::string& value);
    
    bool hexists(const std::string& key, const std::string& field);
    
    bool hdel(const std::string& key, const std::string& field);
    
    std::unordered_map<std::string, std::string> hgetall(const std::string& key);
    
    std::vector<std::string> hkeys(const std::string& key);
    
    std::vector<std::string> hvals(const std::string& key);
    
    ssize_t hlen(const std::string& key);
    
    bool hmset(const std::string& key, const std::vector<std::pair<std::string, std::string>>& fieldValues);

private:
    VioletDB()  = default;
    ~VioletDB() = default;

    VioletDB(const VioletDB&)             = delete;
    VioletDB& operator=(const VioletDB&)  = delete;




private:
    std::mutex m_dbMutex;
    
    std::unordered_map<std::string ,std::string> m_kvStore;
    std::unordered_map<std::string, std::vector<std::string>> m_listStore; 
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_hashStore;
    std::unordered_map<std::string,std::chrono::steady_clock::time_point> m_expiryMap;
};

#endif
