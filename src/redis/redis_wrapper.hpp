#ifndef REDIS_WRAPPER_HPP
#define REDIS_WRAPPER_HPP

#include <sw/redis++/redis++.h>
#include <string>
#include <optional>

class IRedisWrapper {
public:
    virtual ~IRedisWrapper() = default;
    virtual std::optional<std::string> get(const std::string& key) = 0;
    virtual void set(const std::string& key, const std::string& value) = 0;
};

class RedisWrapper : public IRedisWrapper {
public:
    RedisWrapper(const std::string& host = "127.0.0.1", int port = 6379);
    void set(const std::string& key, const std::string& value) override;
    std::optional<std::string> get(const std::string& key) override;

private:
    std::shared_ptr<sw::redis::Redis> redis_;
};

#endif