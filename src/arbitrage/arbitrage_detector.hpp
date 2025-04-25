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
constexpr int nOfPairs = nOfCurrencies * (nOfCurrencies - 1);
constexpr int nOfTriplets = nOfPairs * (nOfCurrencies - 2);

typedef std::vector<std::vector<double>> ExchangeRateGraph;
typedef std::pair<int,int> ExchangeRateEdge;
enum ExchangeCostUpdateResponse { SUCCESS, FAIL };

class ArbitrageDetector {
public:
    ArbitrageDetector(std::shared_ptr<IRedisWrapper> _redisClient);
    void pullGraph(std::vector<int>& encodedIndices);
    int findArbitrages(std::vector<int>& encodedIndices);
    std::vector<int> decodeIndexPair(int index);
    std::vector<int> decodeIndexTriplet(int index);
    ExchangeRateGraph getGraph() { return graph; }
private:
    std::shared_ptr<IRedisWrapper> redisClient;
    ExchangeRateGraph graph;
    bool checkArbitrageOpportunity(std::vector<int>& currencies);
    void printArbitrage(std::vector<int>& currencies);
};

#endif