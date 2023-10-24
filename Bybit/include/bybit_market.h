#pragma once
#include "ws_client.h"
#include "market.h"
#include "datas.h"
#include "pimpl.h"

using MarketOperateResult = core::base::datas::MarketOperateResult;


class BybitMarket final : public core::api::market::Market {
public:
    BybitMarket();
    ~BybitMarket();

    bool is_ready() override;

    MarketOperateResult subscribe() override;
    MarketOperateResult unsubscribe() override;

private:
    void on_open();
    void on_close();
    void on_message(std::string const &msg);

private:
    Self &self;
};