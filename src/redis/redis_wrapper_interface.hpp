#ifndef REDIS_WRAPPER_INTERFACE_HPP
#define REDIS_WRAPPER_INTERFACE_HPP

#include <sw/redis++/redis++.h>
#include <string>
#include <optional>

class RedisWrapperInterface {
public:
    virtual ~RedisWrapperInterface() = default;
    virtual std::optional<std::string> get(const std::string& key) = 0;
    virtual void set(const std::string& key, const std::string& value) = 0;
};

#endif