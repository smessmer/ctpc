#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

constexpr auto success() {
    return [] (Input input) -> ParseResult<std::nullptr_t> {
        return ParseResult<std::nullptr_t>::success(input, nullptr);
    };
}

constexpr auto failure() {
    return [] (Input input) -> ParseResult<std::nullptr_t> {
        return ParseResult<std::nullptr_t>::failure(input);
    };
}

constexpr auto error() {
    return [] (Input input) -> ParseResult<std::nullptr_t> {
        return ParseResult<std::nullptr_t>::error(input);
    };
}

}
