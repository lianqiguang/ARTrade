#include <iostream>
#include <ctime>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include <core/util.h>
#include "bybit.h"

using namespace std::chrono_literals;

int main(int argc, char **argv) {
    std::string Project = "Bybit";
    if (core::util::cli_parse(Project, argc, argv) != 0) {
        spdlog::error("[{}] Parameter parsing failed!", Project);
        exit(-1);
    }

    Bybit bybit;
    do {
        spdlog::info("bybit is market ready? {}", bybit.is_market_ready() ? " true" : "false");
        std::this_thread::sleep_for(1000ms);
    } while (!bybit.is_market_ready());

    std::this_thread::sleep_for(1000ms);
    bybit.market().subscribe({"tickers.BTCUSDT"});
    bybit.run();
    return 0;
}