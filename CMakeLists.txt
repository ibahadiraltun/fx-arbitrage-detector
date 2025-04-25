cmake_minimum_required(VERSION 3.17)
project(fx_arbitrage_detector)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

message("--- Finding packages using pkg-config..")
find_package(PkgConfig REQUIRED)
pkg_check_modules(REDIS_PLUS_PLUS REQUIRED IMPORTED_TARGET redis++)
pkg_check_modules(HIREDIS REQUIRED IMPORTED_TARGET hiredis)
pkg_check_modules(RDKAFKA REQUIRED IMPORTED_TARGET rdkafka)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
message("--- Checked all required packages.")

message("--- Finding OpenSSL..")
find_package(OpenSSL REQUIRED)
message("--- Found OpenSSL.")

message("--- Finding GTest..")
find_package(GTest REQUIRED)
message("--- Found GTest.")

message("--- Adding executables..")
add_executable(kafka_consumer
    src/kafka/consumer/exchange_rate_consumer.cpp
    src/redis/redis_wrapper.cpp
)
target_link_libraries(kafka_consumer
    PRIVATE
    PkgConfig::RDKAFKA
    PkgConfig::REDIS_PLUS_PLUS
    PkgConfig::HIREDIS
    PkgConfig::GLIB
    OpenSSL::SSL
    OpenSSL::Crypto
)

add_executable(fx_arbitrage_detector
    src/arbitrage/main.cpp
    src/arbitrage/arbitrage_detector.cpp
    src/redis/redis_wrapper.cpp
)
target_link_libraries(fx_arbitrage_detector
    PRIVATE
    PkgConfig::REDIS_PLUS_PLUS
    PkgConfig::HIREDIS
    PkgConfig::GLIB
    OpenSSL::SSL
    OpenSSL::Crypto
)

add_executable(ArbitrageDetectorTest
    tests/arbitrage_detector_tests.cpp
    src/arbitrage/arbitrage_detector.cpp
    src/redis/redis_wrapper.cpp
)
target_link_libraries(ArbitrageDetectorTest
    PRIVATE
    PkgConfig::REDIS_PLUS_PLUS
    PkgConfig::HIREDIS
    GTest::gtest_main
    OpenSSL::SSL
    OpenSSL::Crypto
)
message("--- All executables are added.")

message("--- Adding tests..")
add_test(NAME ArbitrageDetectorTest COMMAND ArbitrageDetectorTest)
message("--- All tests are added.")
