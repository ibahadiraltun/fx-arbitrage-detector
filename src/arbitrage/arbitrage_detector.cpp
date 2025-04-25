#include "arbitrage_detector.hpp"
#include <iostream>
#include <format>

ArbitrageDetector::ArbitrageDetector(std::shared_ptr<IRedisWrapper> _redisClient) : redisClient(_redisClient) {
    graph = std::vector<std::vector<double>>(nOfCurrencies, std::vector<double>(nOfCurrencies, DOUBLE_MAX));
};

std::vector<int> ArbitrageDetector::decodeIndexPair(int index) {
    int startIndex = index / (nOfCurrencies - 1);
    int endIndex = index % (nOfCurrencies - 1);
    if (endIndex >= startIndex) {
        endIndex++;
    }
    return {startIndex, endIndex};
}

std::vector<int> ArbitrageDetector::decodeIndexTriplet(int index) {
    int iters = index / (nOfCurrencies - 2);
    int startIndex = iters / (nOfCurrencies - 1);
    int middleIndex = iters % (nOfCurrencies - 1);
    int endIndex = index % (nOfCurrencies - 2);

    if (middleIndex >= startIndex) {
        middleIndex++;
    }
    if (endIndex >= std::min(startIndex, middleIndex)) {
        endIndex++;
    }
    if (endIndex >= std::max(startIndex, middleIndex)) {
        endIndex++;
    }
    return {startIndex, middleIndex, endIndex};
}

void ArbitrageDetector::pullGraph(std::vector<int>& encodedIndices) {
    if (!redisClient) {
        std::cerr << "No redis cache is provided for pulling the graph.\n";
        throw;
    }
    for (auto index : encodedIndices) {
        auto currencyPair = decodeIndexPair(index);
        auto res = redisClient->get(CURRENCIES[currencyPair[0]] + ":" + CURRENCIES[currencyPair[1]]);
        if (res) {
            graph[currencyPair[0]][currencyPair[1]] = -log(atof(res->c_str()));
        }
    }
}

void ArbitrageDetector::printArbitrage(std::vector<int>& currencies) {
    std::cout << "Arbitrage detected @ ";
    double profit = 0;
    int previousCurrencyIdx = -1;
    for (int currencyIdx : currencies) {
        std::cout << CURRENCIES[currencyIdx] << " -> ";
        if (previousCurrencyIdx != -1) {
            profit += graph[previousCurrencyIdx][currencyIdx];
        }
        previousCurrencyIdx = currencyIdx;
    }
    profit += graph[previousCurrencyIdx][currencies[0]];
    std::cout << CURRENCIES[currencies[0]] << " | Profit = " << std::format("{:.9f}%", 100.0 * (std::exp(-profit) - 1.0)) << '\n';
}

bool ArbitrageDetector::checkArbitrageOpportunity(std::vector<int>& currencies) {
    if (currencies.size() <= 1) {
        std::cerr << "Arbitrage requires at least two currencies.\n";
        return false;
    }
    double profit = 0;
    int numCurrency = currencies.size();
    for (int i = 1; i < numCurrency + 1; i++) {
        double rate = graph[currencies[i - 1]][currencies[i % numCurrency]];
        if (rate == DOUBLE_MAX) {
            return false;
        }
        profit += rate;
    }
    return profit < 0;
}

int ArbitrageDetector::findArbitrages(std::vector<int>& encodedIndices) {
    int cnt = 0;
    for (auto index : encodedIndices) {
        auto currencies = decodeIndexTriplet(index);
        if (checkArbitrageOpportunity(currencies)) {
            printArbitrage(currencies);
            cnt++;
        }
    }
    return cnt;
}
