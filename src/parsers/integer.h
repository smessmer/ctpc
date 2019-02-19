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
            [] (char digit) {
                return digit - '0';
            }
    );
}

constexpr auto integer() {
    return map(
            rep1(digit()),
            [] (auto digits) {
                long long result = 0;
                for (auto digit : digits) {
                    result *= 10;
                    result += digit;
                }
                return result;
            }
    );
}

}
