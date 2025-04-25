# FOREX Arbitrage Detection

Purpose of this project is detecting triplet arbitrages between [36 most traded currencies](https://en.wikipedia.org/wiki/Foreign_exchange_market#:~:text=foreign%20exchange%20companies.-,Most%20traded%20currencies%20by%20value,-Most%20traded%20currencies). Detecting single arbitrage on average costs less than **5µs** if multithreading is utilized, and 50µs if processed in a single thread.

## Components

### Kafka
Kafka is used for fetching exchange rates from public API and producing them into a single topic with 36 partitions. On the otherhand, consumer subscribes into this topic, consumes messages, and store them in Redis Cache.

Latency of this component could be improved significantly if more computational resources are used. For instance, using 36 different processes for fetching data (each for a single currency), producing messages to 36 topics (again for each single currency),
and finally, 36 consumer processes to read only a single topic. However, this project is particularly focused on the performance of the Arbitrage component while making sure we have an end-to-end application.

Configuration of the Kafka Cluster:
- Single broker (thus, replication factor is 1)
- Single producer and consumer
- Single topic with 36 partitions

### Redis
Each message consumed in Kafka component written into Redis Cache. In order to handle Redis actions both in the Arbitrage and Kafka components, class RedisWrapper is implemented on top of [Redis++](https://github.com/sewenew/redis-plus-plus) and supports two basic SET and GET operations.

### Arbitrage
This component does not use Bellman-Ford nor any linear algebra for detecting arbitrages. It simply checks all possible triplets for arbitrage opportunity.

Between 36 currencies, there are 36 * 35 * 34 = 42840 triplets. Each of these triplets are checked for arbitrage opportunity using the rates pulled from Redis Cache. To prevent arithmetic underflow, each rate is stored in natural logarithm.

**Multithreading** is supported and impacts performance by **x10**. To utilize faster and more memory-optimized threading, each of the candidate pairs and triplets are encoded.  For example:  
`
  Encode(0-1-2) = 0,
  Encode(0-1-3) = 1,
  ...,
  Encode(35, 34, 33) = 42739
`  
`
  Encode(0-1) = 0,
  Encode(0-2) = 1,
  ...,
  Encode(35, 34) = 1259
`

Two major functionality based on this:
1) `pullGraph(std::vector<int>& encodedIndices)`:  
    <sub>Parameter is expected to be encoded by currency pairs. For example `USD:EUR` maps to `0-1` which is encoded as `0`.</sub>  
    <sub>Function decodes this index and pulls the pair's exchange rate from Redis Cache.</sub>  
    <sub>Default configutation uses significantly more number of threads than hardware concurrency, in order to reduce overhead due to query switches for Redis.
2) `findArbitrages(std::vector<int>& encodedIndices)`:  
    <sub>Parameter is expected to be encoded by currency triplets. For example `USD:EUR:JPY` maps to `0-1-2` which is encoded as `0`.</sub>  
    <sub>Function decodes this index and checks if the triplet has arbitrage opportunity.</sub>  
    <sub>Since this function is CPU-only, default configutation uses similar number of threads to hardware concurrency.

## Installation
This is tested on macOS Sequoia 15.4. Please make sure you have installed `brew` as package manager. Then,
```
brew install pkgconf cmake make
```
Once initial these are installed, you may clone and build the application:
```
git clone https://github.com/ibahadiraltun/fx-arbitrage-detector.git
cd fx-arbitrage-detector
mkdir build && cd build
cmake ..
make
```

**Note**: Kafka producer is implemented in Python, therefore, you need to install following libraries:
```
pip install confluent-kafka requests
```
**Running:**
- Kafka producer could be run using `cd src/kafka/producer && python exchange_rate_producer.py`.  
- Kafka consumer is compiled and ready to execute in build by `cd build && ./kafka_consumer`.  
- Arbitrage component is compiled and ready to execute by `cd build && ./fx_arbitrage_detector`.

## Examples
<img width="724" alt="Screenshot 2025-04-23 at 16 14 45" src="https://github.com/user-attachments/assets/12a3ba62-922a-43e4-b3bd-b8a0b0db84a1" />

