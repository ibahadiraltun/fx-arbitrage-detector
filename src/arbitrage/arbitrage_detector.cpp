#include "arbitrage_detector.hpp"
#include <iostream>
#include <format>

ArbitrageDetector::ArbitrageDetector(std::shared_ptr<IRedisWrapper> _redisClient) : redisClient(_redisClient) {
    graph = std::vector<std::vector<double>>(nOfCurrencies, std::vector<double>(nOfCurrencies, DOUBLE_MAX));
    exchangeCost = std::vector<double>(nOfCurrencies, DOUBLE_MAX);
    previousCurrency = std::vector<int>(nOfCurrencies, -1);
};

void ArbitrageDetector::pullGraph() {
    if (!redisClient) {
        std::cerr << "No redis cache is provided for pulling the graph.\n";
        throw;
    }
    edges.clear();
    for (int sourceIdx = 0; sourceIdx < nOfCurrencies; ++sourceIdx) {
        for (int destinationIdx = 0; destinationIdx < nOfCurrencies; ++destinationIdx) {
            if (sourceIdx == destinationIdx) {
                continue;
            }
            auto res = redisClient->get(CURRENCIES[sourceIdx] + ":" + CURRENCIES[destinationIdx]);
            if (res) {
                graph[sourceIdx][destinationIdx] = -log(atof(res->c_str()));
                edges.push_back({sourceIdx, destinationIdx});
            }
        }
    }
}

ExchangeCostUpdateResponse ArbitrageDetector::updateExchangeCost(ExchangeRateEdge& edge, bool updateParent) {
    auto [source, destination] = edge;
    double candidateCost = (exchangeCost[source] == DOUBLE_MAX) ? DOUBLE_MAX : exchangeCost[source] + graph[source][destination];
    if (candidateCost < exchangeCost[destination]) {
        exchangeCost[destination] = candidateCost;
        if (updateParent) {
            previousCurrency[destination] = source;
        }
        return ExchangeCostUpdateResponse::SUCCESS;
    }
    return ExchangeCostUpdateResponse::FAIL;
}

void ArbitrageDetector::printArbitrage(std::vector<int> currencies) {
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

void ArbitrageDetector::runArbitrageDetector() {
    auto start = std::chrono::system_clock::now();
    int cnt = 0;
    for (int i = 0; i < nOfCurrencies; i++) {
        for (int j = 0; j < nOfCurrencies; j++) {
            if (i == j) {
                continue;
            }
            for (int k = 0; k < nOfCurrencies; k++) {
                if (i == k || j == k) {
                    continue;
                }
                double cost = graph[i][j] + graph[j][k] + graph[k][i];
                if (cost < 0) {
                    printArbitrage({i, j, k});
                    cnt++;
                }
            }
        }
    }
    auto end = std::chrono::system_clock::now();
    std::cerr << "time elapsed === " << end - start << " for " << cnt << " arbitrages" << '\n';
}
