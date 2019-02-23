#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

template<class MatchFunction>
constexpr auto elem(MatchFunction&& match) {
    return [match = std::forward<MatchFunction>(match)] (Input input) -> ParseResult<char> {
        if (input.input.size() == 0) {
            return ParseResult<char>::failure(input);
        } else if (match(input.input[0])) {
            return ParseResult<char>::success(Input{input.input.substr(1)}, input.input[0]);
        } else {
            return ParseResult<char>::failure(input);
        }
    };
}

constexpr auto elem(char expected) {
    return elem([expected] (char v) {return v == expected;});
}

}
