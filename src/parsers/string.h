#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

// TODO This parser takes the expected string as a string_view, i.e. doesn't take ownership.
//      Replace this with some owning compile-time string mechanism.
constexpr auto string(std::string_view expected) {
    return [expected] (Input input) -> ParseResult<std::string_view> {
        size_t i = 0;
        for(; i < expected.size(); ++i) {
            if (expected[i] != input.input[i]) {
                return ParseResult<std::string_view>::failure(Input{input.input.substr(i)});
            }
        }
        return ParseResult<std::string_view>::success(input.input.substr(0, i), Input{input.input.substr(i)});
    };
}

}
