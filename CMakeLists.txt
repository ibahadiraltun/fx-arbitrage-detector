cmake_minimum_required(VERSION 3.17)
project(fx_arbitrage_detector)

enable_testing()

set(CMAKE_CXX_STANDARD 20)

include_directories(/opt/homebrew/include)

link_directories(/usr/local/lib)
link_directories(/opt/homebrew/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/opt/homebrew/Cellar/librdkafka/2.8.0/include -I/opt/homebrew/Cellar/openssl@3/3.4.1/include -I/opt/homebrew/opt/zstd/include -I/opt/homebrew/opt/lz4/include -L/opt/homebrew/Cellar/librdkafka/2.8.0/lib -lrdkafka")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/opt/homebrew/Cellar/glib/2.84.1/include/glib-2.0 -I/opt/homebrew/Cellar/glib/2.84.1/lib/glib-2.0/include -I/opt/homebrew/opt/gettext/include -I/opt/homebrew/Cellar/pcre2/10.45/include -L/opt/homebrew/Cellar/glib/2.84.1/lib -lglib-2.0 -L/opt/homebrew/opt/gettext/lib -lintl")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/opt/homebrew/Cellar/hiredis/1.2.0/include/hiredis -I/opt/homebrew/Cellar/hiredis/1.2.0/include -D_FILE_OFFSET_BITS=64 -L/opt/homebrew/Cellar/hiredis/1.2.0/lib -lhiredis")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include -I/opt/homebrew/Cellar/hiredis/1.2.0/include/hiredis -I/opt/homebrew/Cellar/hiredis/1.2.0/include -D_FILE_OFFSET_BITS=64 -L/usr/local/lib -lredis++ -L/opt/homebrew/Cellar/hiredis/1.2.0/lib -lhiredis")

add_executable(kafka_consumer
    src/kafka/consumer/exchange_rate_consumer.cpp
    src/redis/redis_wrapper.cpp
)
target_link_libraries(kafka_consumer PRIVATE redis++ hiredis)

add_executable(fx_arbitrage_detector
    src/arbitrage/main.cpp
    src/arbitrage/arbitrage_detector.cpp
    src/redis/redis_wrapper.cpp
)
target_link_libraries(fx_arbitrage_detector PRIVATE redis++ hiredis)

add_executable(ArbitrageDetectorTest
    tests/arbitrage_detector_tests.cpp
    src/arbitrage/arbitrage_detector.cpp
    src/redis/redis_wrapper.cpp
)
target_link_libraries(ArbitrageDetectorTest PRIVATE redis++ hiredis PUBLIC gtest_main)
add_test(NAME ArbitrageDetectorTest COMMAND ArbitrageDetectorTest)
