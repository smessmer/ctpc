#pragma once

#include "parsers/parse_result.h"
#include "parsers/elem.h"
#include "parsers/map.h"
#include "parsers/rep.h"

namespace ctpc {

constexpr auto numeric() {
    return elem([] (char v) {
        return v >= '0' && v <= '9';
    });
}

constexpr auto digit() {
    return map(
            numeric(),
            [] (char digit) -> uint8_t {
                return digit - '0';
            }
    );
}

constexpr auto integer() {
    return rep1(
        digit(),
        [] () -> int64_t {return 0;},
        [] (int64_t* accumulator, uint8_t digit) {
            *accumulator *= 10;
            *accumulator += digit;
        }
    );
}

}
