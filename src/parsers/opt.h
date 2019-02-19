#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

// TODO Test this moves parser correctly (i.e. works with movable-only and also count number of moves/copies)
template<class Parser>
constexpr auto opt(Parser parser) {
    using parse_result = ParseResult<std::optional<parser_result_t<Parser>>>;
    return [parser = std::move(parser)] (Input input) -> parse_result {
        auto parsed = parser(input);
        if (parsed.is_success()) {
            return parse_result::success(parsed.result(), parsed.next());
        } else {
            return parse_result::success(std::nullopt, input);
        }
    };
}

}
