#ifndef REDIS_WRAPPER_HPP
#define REDIS_WRAPPER_HPP

#include <sw/redis++/redis++.h>
#include <string>
#include <optional>
#include "redis_wrapper_interface.hpp"

class RedisWrapper : public RedisWrapperInterface {
public:
    RedisWrapper(const std::string& host = "127.0.0.1", int port = 6379);
    void set(const std::string& key, const std::string& value) override;
    std::optional<std::string> get(const std::string& key) override;

private:
    std::shared_ptr<sw::redis::Redis> redis_;
};

#endif