#include "arbitrage_detector.hpp"

int main() {
    std::shared_ptr<IRedisWrapper> redisClient = std::make_shared<RedisWrapper>("127.0.0.1", 6379);
    ArbitrageDetector detector(redisClient);
    while (true) {
        detector.pullGraph();
        detector.runArbitrageDetector();
    }

    return 0;
}