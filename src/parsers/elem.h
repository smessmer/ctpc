#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

template<class MatchFunction>
// TODO Test that this moves/copies MatchFunction correctly (i.e. works with movable-only and also count number of moves/copies)
constexpr auto elem(MatchFunction match) {
    return [match = std::move(match)] (Input input) -> ParseResult<char> {
        if (input.input.size() == 0) {
            return ParseResult<char>::failure(input);
        } else if (match(input.input[0])) {
            return ParseResult<char>::success(input.input[0], Input{input.input.substr(1)});
        } else {
            return ParseResult<char>::failure(input);
        }
    };
}

constexpr auto elem(char expected) {
    return elem([expected] (char v) {return v == expected;});
}

}
