#include <unordered_map>
#include <gtest/gtest.h>
#include <limits.h>
#include "../src/arbitrage/arbitrage_detector.hpp"

class FakeRedisWrapper : public IRedisWrapper {
public:
    FakeRedisWrapper() : data({}) {}
    void set(const std::string& key, const std::string& value) override {
        data[key] = value;
    }
    std::optional<std::string> get(const std::string& key) override {
        if (data.find(key) != data.end()) {
            return data[key];
        }
        return std::nullopt;
    }
private:
    std::unordered_map<std::string,std::string> data;
};  

TEST(ArbitrageDetectorTest, DefaultConstructor) {
    std::shared_ptr<IRedisWrapper> redisClient = std::make_shared<FakeRedisWrapper>();
    ArbitrageDetector detector(redisClient);
    EXPECT_EQ(nOfCurrencies, detector.getGraph().size());
    EXPECT_EQ(nOfCurrencies, detector.getGraph()[0].size());
    for (int i = 0; i < nOfCurrencies; i++) {
        for (int j = 0; j < nOfCurrencies; j++) {
            EXPECT_EQ(DOUBLE_MAX, detector.getGraph()[i][j]);
        }
    }
}

TEST(ArbitrageDetectorTest, DecodeIndexPair) {
    std::shared_ptr<IRedisWrapper> redisClient = std::make_shared<FakeRedisWrapper>();
    ArbitrageDetector detector(redisClient);
    int cnt = 0;
    for (int i = 0; i < nOfCurrencies; i++) {
        for (int j = 0; j < nOfCurrencies; j++) {
            if (i == j) {
                continue;
            }
            EXPECT_EQ(std::vector<int>({i, j}), detector.decodeIndexPair(cnt++));
        }
    }
}

TEST(ArbitrageDetectorTest, DecodeIndexTriplet) {
    std::shared_ptr<IRedisWrapper> redisClient = std::make_shared<FakeRedisWrapper>();
    ArbitrageDetector detector(redisClient);
    int cnt = 0;
    for (int i = 0; i < nOfCurrencies; i++) {
        for (int j = 0; j < nOfCurrencies; j++) {
            if (i == j) {
                continue;
            }
            for (int k = 0; k < nOfCurrencies; k++) {
                if (k == i || k == j) {
                    continue;
                }
                EXPECT_EQ(std::vector<int>({i, j, k}), detector.decodeIndexTriplet(cnt++));
            }
        }
    }
}

TEST(ArbitrageDetectorTest, PullGraph) {
    std::shared_ptr<IRedisWrapper> redisClient = std::make_shared<FakeRedisWrapper>();
    ArbitrageDetector detector(redisClient);
    int fakeRate = 0;
    std::vector<std::vector<double>> data(nOfCurrencies, std::vector<double>(nOfCurrencies, 0));
    for (int i = 0; i < nOfCurrencies; i++) {
        for (int j = 0; j < nOfCurrencies; j++) {
            std::string key = CURRENCIES[i] + ":" + CURRENCIES[j];
            data[i][j] = -log(fakeRate);
            redisClient->set(key, std::to_string(fakeRate));
            fakeRate += 0.1;
        }
    }
    int cnt = 0;
    for (int i = 0; i < nOfCurrencies; i++) {
        for (int j = 0; j < nOfCurrencies; j++) {
            if (i != j) {
                std::vector<int> tmp = {cnt++};
                detector.pullGraph(tmp);
                EXPECT_EQ(data[i][j], detector.getGraph()[i][j]);
            }
        }
    }
}

TEST(ArbitrageDetectorTest, FindArbitragesEmpty) {
    std::shared_ptr<IRedisWrapper> redisClient = std::make_shared<FakeRedisWrapper>();
    ArbitrageDetector detector(redisClient);
    std::vector<int> idx = {0};
    detector.pullGraph(idx);
    testing::internal::CaptureStdout();
    detector.findArbitrages(idx);
    std::string expected = "";
    EXPECT_EQ(expected, testing::internal::GetCapturedStdout());
}

TEST(ArbitrageDetectorTest, FindArbitragesSingle) {
    std::shared_ptr<IRedisWrapper> redisClient = std::make_shared<FakeRedisWrapper>();
    ArbitrageDetector detector(redisClient);
    redisClient->set("usd:eur", "0.5"); // 0-1
    redisClient->set("eur:jpy", "1.3"); // 1-2
    redisClient->set("jpy:usd", "1.7"); // 2-0
    std::vector<int> idx = {0, nOfCurrencies, (nOfCurrencies - 1) * 2};
    detector.pullGraph(idx);
    testing::internal::CaptureStdout();
    idx = {0};
    detector.findArbitrages(idx);
    std::string expected = "Arbitrage detected @ usd -> eur -> jpy -> usd | Profit = 10.500000000%\n";
    EXPECT_EQ(expected, testing::internal::GetCapturedStdout());
}

TEST(ArbitrageDetectorTest, FindArbitragesMultiple) {
    std::shared_ptr<IRedisWrapper> redisClient = std::make_shared<FakeRedisWrapper>();
    ArbitrageDetector detector(redisClient);
    redisClient->set("usd:eur", "0.5"); // 0-1
    redisClient->set("eur:jpy", "1.3"); // 1-2
    redisClient->set("jpy:usd", "1.7"); // 2-0
    std::vector<int> idx = {0, nOfCurrencies, (nOfCurrencies - 1) * 2};
    detector.pullGraph(idx);
    testing::internal::CaptureStdout();
    idx = {0, nOfCurrencies * (nOfCurrencies - 2), 2 * (nOfCurrencies - 1) * (nOfCurrencies - 2)}; // 0-1-2, 1-2-0, 2-0-1
    detector.findArbitrages(idx);
    std::string expected = "Arbitrage detected @ usd -> eur -> jpy -> usd | Profit = 10.500000000%\n";
    expected += "Arbitrage detected @ eur -> jpy -> usd -> eur | Profit = 10.500000000%\n";
    expected += "Arbitrage detected @ jpy -> usd -> eur -> jpy | Profit = 10.500000000%\n";
    EXPECT_EQ(expected, testing::internal::GetCapturedStdout());
}
