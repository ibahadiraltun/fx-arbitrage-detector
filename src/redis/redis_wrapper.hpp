#ifndef REDIS_WRAPPER_HPP
#define REDIS_WRAPPER_HPP

#include <sw/redis++/redis++.h>
#include <string>
#include <optional>

class RedisWrapper {
public:
    RedisWrapper(const std::string& host = "127.0.0.1", int port = 6379);
    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);

private:
    std::shared_ptr<sw::redis::Redis> redis_;
};

#endif