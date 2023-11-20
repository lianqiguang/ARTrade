#include <iostream>
#include <ctime>
#include <filesystem>

#include "core/modules.h"
#include "core/config.h"
#include "core/util.h"
#include "core/message.hpp"


#define LOGHEAD "[Modules::" + std::string(__func__) + "]"

namespace {

struct Self {
    core::api::market::Market *market = nullptr;
    core::api::trade::Trade *trade = nullptr;
    core::message::Message *message = nullptr;

    ~Self() {
        if (market) { delete market; }
        if (trade) { delete trade; }
        if (message) { delete message; }
    }
};

}

namespace core::modules {

Modules::Modules(int argc, char** argv) : self { *new Self{} } {
    // default setting
    init(argc, argv);
}

Modules::~Modules() {
    if (&self) { delete &self; }
}

void Modules::init(int argc, char** argv, size_t log_size, size_t log_files) {
    core::util::startup(this, argc, argv, log_size, log_files);
}

// Market
void Modules::set_market_obj(core::api::market::Market *market_obj) {
    if (self.market) {
        delete self.market;
    }
    self.market = market_obj;
}

bool Modules::is_market_ready() {
    if (self.market) {
        return self.market->is_ready();
    }

    return false;
}

// Trade
void Modules::set_trade_obj(core::api::trade::Trade *trade_obj) {
    if (self.trade) {
        delete self.trade;
    }
    self.trade = trade_obj;
}

bool Modules::is_trade_ready() {
    if (self.trade) {
        return self.trade->is_ready();
    }
    return false;
}

// Config
void Modules::init_config() {
    default_config();
    spdlog::info("{} init config with default config!", LOGHEAD);
}

void Modules::default_config() {
    nlohmann::json default_config = nlohmann::json::parse(
        R"(
            {
                "project": "default",
                "version": 1.0
            }
        )"
    );

    core::config::Config::init(default_config);
}

// 
bool Modules::is_ready() {
    if (self.trade && self.market) {
        return self.trade->is_ready() && self.market->is_ready();
    } else if (self.trade) {
        return self.trade->is_ready();
    } else if (self.market) {
        return self.market->is_ready();
    } 
    return false;
}

void handle_sigint(int signal) {
    std::cout << std::endl;
    spdlog::info("{} Received Ctrl+C signal, program is about to exit!", LOGHEAD);
    
    exit(signal);
}

void Modules::custom_init() {
    // something init after custom modules init

    // config
    init_config();
    core::util::create_folder(core::util::config_path());
    bool read_config_status = core::config::Config::read(
        (core::util::config_path() / std::filesystem::path("app.json")).string()
    );

    if (read_config_status == false) {
        spdlog::error("{} cannot found 'app.json' at {}", LOGHEAD, core::util::config_path().string());
        exit(-3);
    }
    
    core::base::datas::MessageType type = message_type();
    std::string proj = project_name();
    self.message = new core::message::Message(proj, type, Identity::Master);
    if (self.market) { self.market->set_message(self.message); }
    if (self.trade) { self.trade->set_message(self.message); }
}


void Modules::run() {
    std::signal(SIGINT, handle_sigint);

    custom_init();

    // check if it is ready
    do {
        spdlog::info("modules is ready? {}", is_ready() ? "true" : "false");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!is_ready());

    auto ts = std::chrono::system_clock::now();

    core::base::datas::CommandObj* command = nullptr;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        auto now = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - ts) >= std::chrono::seconds(1)) {
            interval_1s();
            ts = now;
        }

        // read commands & deal with commands
        command = self.message->read_command();
        if (command) {
            if (command->type == CommandType::SUBSCRIBE) {
                std::vector<std::string> symbols;
                for (auto item: command->symbols.symbols) {
                    if (std::strlen(item) > 0) {
                        symbols.emplace_back(item);
                    }
                }

                if (symbols.size() > 0) {
                    self.market->subscribe(std::move(symbols));
                }
            } else if (command->type == CommandType::UNSUBSCRIBE) {
                std::vector<std::string> symbols;
                for (auto item: command->symbols.symbols) {
                    if (std::strlen(item) > 0) {
                        symbols.emplace_back(item);
                    }
                }

                if (symbols.size() > 0) {
                    self.market->unsubscribe(std::move(symbols));
                }
            } else if (command->type == CommandType::ORDER) {

            } else if (command->type == CommandType::CANCEL) {

            } else {
                spdlog::error("{} type: {}", LOGHEAD, static_cast<char>(command->type));
            }
        }
    }
}

void Modules::interval_1s() {
    spdlog::info("{}", LOGHEAD);
}

} // namespace core::modules

#undef LOGHEAD