#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

template<class Parser>
constexpr auto opt(Parser&& parser) {
    using parse_result = ParseResult<std::optional<parser_result_t<Parser>>>;
    return [parser = std::forward<Parser>(parser)] (Input input) -> parse_result {
        auto parsed = parser(input);
        if (parsed.is_success()) {
            return parse_result::success(parsed.result(), parsed.next());
        } else {
            return parse_result::success(std::nullopt, input);
        }
    };
}

}
