#include "arbitrage_detector.hpp"
#include <thread>
#include <future>
#include <iostream>

int main() {
    std::shared_ptr<IRedisWrapper> redisClient = std::make_shared<RedisWrapper>("127.0.0.1", 6379);
    ArbitrageDetector detector(redisClient);

    std::vector<std::future<void>> threadsPull;
    std::vector<std::future<int>> threadsDetect;
    int p = nOfCurrencies * (nOfCurrencies - 1); // number of threads to pull data (dependent to redis client)
    int q = nOfCurrencies; // number of threads to check arbitrages (CPU only, no dependency)
    std::cout << "max supported parallel threads:" << std::thread::hardware_concurrency() << '\n';
    for (int i = 0; i < p; i++) {
        threadsPull.emplace_back(std::async(std::launch::async, [&detector, i, p]() {
            std::vector<int> decodedIndices;
            int freq = nOfPairs / p;
            for (int j = 0; j < freq; j++) {
                decodedIndices.push_back(j + i * freq);
            }
            detector.pullGraph(decodedIndices);
        }));
    }
    auto start = std::chrono::system_clock::now();
    for (auto &t : threadsPull) {
        t.get();
    }

    for (int i = 0; i < q; i++) {
        threadsDetect.emplace_back(std::async(std::launch::async, [&detector, i, q]() {
            std::vector<int> decodedIndices;
            int freq = nOfTriplets / q;
            for (int j = 0; j < freq; j++) {
                decodedIndices.push_back(j + i * freq);
            }
            return detector.findArbitrages(decodedIndices);
        }));
    }

    int numArbitrages = 0;
    for (auto &t : threadsDetect) {
        numArbitrages += t.get();
    }
    auto end = std::chrono::system_clock::now();
    std::cerr << "found " << numArbitrages << " arbitrages in " << end - start << '\n';

    return 0;
}