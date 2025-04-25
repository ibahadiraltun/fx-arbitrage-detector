#include "redis_wrapper.hpp"
#include <iostream>

RedisWrapper::RedisWrapper(const std::string& host, int port) {
    try {
        sw::redis::ConnectionOptions connection_options;
        connection_options.host = host;
        connection_options.port = port;
        connection_options.connect_timeout = std::chrono::milliseconds(200);
        redis_ = std::make_shared<sw::redis::Redis>(connection_options);
    } catch (const std::exception &err) {
        std::cerr << "Redis initialization is failed to connect [" << host << "] @ " << port << '\n';
        throw;
    }
}

void RedisWrapper::set(const std::string& key, const std::string& value) {
    try {
        redis_->set(key, value);
    } catch (const std::exception &err) {
        std::cerr << "Redis SET is failed for key = " << key << ", value = " << value << '\n';
    }
}

std::optional<std::string> RedisWrapper::get(const std::string& key) {
    try {
        return redis_->get(key);
    } catch (const std::exception &err) {
        std::cerr << "Redis GET is failed for key = " << key << '\n';
    }
}