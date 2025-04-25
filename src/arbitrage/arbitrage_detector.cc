#include "arbitrage_detector.hpp"
#include <iostream>
#include <format>

ArbitrageDetector::ArbitrageDetector(std::shared_ptr<RedisWrapper> _redisClient) : redisClient(_redisClient) {
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

void ArbitrageDetector::printArbitrage(int currencyIdx) {
    for (int _ = 0; _ < nOfCurrencies; ++_) {
        currencyIdx = previousCurrency[currencyIdx];
    }
    std::cerr << "[" << std::format("{0:%F_%T}", std::chrono::system_clock::now()) << "] ";
    std::cerr << "Arbitrage detected @ " << CURRENCIES[currencyIdx];
    int previousCurrencyIdx = previousCurrency[currencyIdx];
    double profit = std::exp(-graph[previousCurrencyIdx][currencyIdx]);
    do {
        std::cerr << " <- " << CURRENCIES[previousCurrencyIdx];
        profit *= std::exp(-graph[previousCurrency[previousCurrencyIdx]][previousCurrencyIdx]);
        previousCurrencyIdx = previousCurrency[previousCurrencyIdx];
    } while (previousCurrencyIdx != currencyIdx);
    std::cerr << " <- " << CURRENCIES[currencyIdx];
    std::cerr << " | Profit = " << std::format("{:.5f}", profit) << "%\n";
}

void ArbitrageDetector::runArbitrageDetector() {
    std::fill(exchangeCost.begin(), exchangeCost.end(), DOUBLE_MAX);
    std::fill(previousCurrency.begin(), previousCurrency.end(), -1);
    exchangeCost[designatedRoot] = 0;
    for (int _ = 0; _ < nOfCurrencies - 1; ++_) {
        for (ExchangeRateEdge& edge : edges) {
            updateExchangeCost(edge);
        }
    }

    for (ExchangeRateEdge& edge : edges) {
        ExchangeCostUpdateResponse res = updateExchangeCost(edge, false);
        if (res == ExchangeCostUpdateResponse::SUCCESS) {
            printArbitrage(edge.second);
        }
    }
}

int main() {
    std::shared_ptr<RedisWrapper> redisClient = std::make_shared<RedisWrapper>("127.0.0.1", 6379);
    ArbitrageDetector detector(redisClient);

    while (true) {
        detector.pullGraph();
        detector.runArbitrageDetector();
    }

    return 0;
}