#ifndef __FILTETER_H__
#define __FILTETER_H__

#include <string>
#include <atomic>
#include <unordered_map>


namespace core::filter {
/***********************************/
/********** market filter **********/
/***********************************/
static std::unordered_map<std::string_view, std::atomic<uint64_t>> SYMBOL_SUBSCRIBED_TIMES;

bool is_need_subscribe(std::string_view symbol) {
    return SYMBOL_SUBSCRIBED_TIMES[symbol].fetch_add(1) > 0;
}

bool is_need_unsubscribe(std::string_view symbol) {
    uint64_t times = SYMBOL_SUBSCRIBED_TIMES[symbol].fetch_sub(1);
    if (times < 0) {
        // Not guaranteed to be 100% thread safe
        SYMBOL_SUBSCRIBED_TIMES[symbol].store(0);
    }
    return times == 0;
}

}

#endif // __FILTETER_H__