#pragma once 
#include "datas.hpp"

namespace core::api::trade {
    
using TradeOperateResult = core::base::datas::TradeOperateResult;

class Trade {
    public:
        Trade() = default;
        virtual ~Trade() = default;

        virtual bool is_ready() = 0;

        virtual TradeOperateResult order() = 0;
        virtual TradeOperateResult cancel() = 0;
};            

} // namespace core::api::trade
