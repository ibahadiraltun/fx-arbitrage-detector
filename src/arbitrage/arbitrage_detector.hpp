#ifndef ARBITRAGE_DETECTOR_HPP
#define ARBITRAGE_DETECTOR_HPP

#include <vector>
#include <string>
#include "../redis/redis_wrapper.hpp"

const std::vector<std::string> CURRENCIES = {
    "usd", "eur", "jpy", "gbp", "cny", "aud", "cad", "chf",
    "hkd", "sgd", "sek", "krw", "nok", "nzd", "inr", "mxn",
    "twd", "zar", "brl", "dkk", "pln", "thb", "ils", "idr",
    "czk", "aed", "try", "huf", "clp", "sar", "php", "myr",
    "cop", "rub", "ron", "pen"
};
constexpr double DOUBLE_MAX = std::numeric_limits<double>::max();
constexpr int nOfCurrencies = 36;

typedef std::vector<std::vector<double>> ExchangeRateGraph;
typedef std::pair<int,int> ExchangeRateEdge;
enum ExchangeCostUpdateResponse { SUCCESS, FAIL };

class ArbitrageDetector {
public:
    ArbitrageDetector(std::shared_ptr<RedisWrapper> _redisClient);
    void pullGraph();
    void runArbitrageDetector(bool bellmanFord = false);
private:
    std::shared_ptr<RedisWrapper> redisClient;
    ExchangeRateGraph graph;
    std::vector<ExchangeRateEdge> edges;
    std::vector<double> exchangeCost;
    std::vector<int> previousCurrency;
    int designatedRoot = 13;

    ExchangeCostUpdateResponse updateExchangeCost(ExchangeRateEdge& edge, bool updateParent = true);
    void runBellmanFord();
    void printArbitrage(int currency);
    void printArbitrage(std::vector<int> currencies);
};

#endif