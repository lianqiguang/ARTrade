#include "market.h"
#include "datas.h"
#include "pimpl.h"

using MarketOperateResult = core::base::datas::MarketOperateResult;


class BinanceMarket final: public core::api::market::Market {
public:
    BinanceMarket();
    ~BinanceMarket();

    bool is_ready() override;

    MarketOperateResult subscribe() override;
    MarketOperateResult unsubscribe() override;

private:
    void on_open();

private:
    Self &self;
};