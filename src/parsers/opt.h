#pragma once

#include "parsers/parse_result.h"
#include "parsers/basic_parsers.h"
#include "parsers/map.h"
#include "parsers/alternative.h"

namespace ctpc {

template<class Parser>
constexpr auto opt(Parser&& parser) {
    using parse_result = ParseResult<std::optional<parser_result_t<Parser>>>;
    return [parser = std::forward<Parser>(parser)] (Input input) -> parse_result {
        auto parsed = parser(input);
        if (parsed.is_success()) {
            Input next = parsed.next();
            return parse_result::success(next, std::move(parsed).result());
        } else if (parsed.is_failure()) {
            return parse_result::success(input, std::nullopt);
        } else {
            ASSERT(parsed.is_error());
            return parse_result::error(parsed.next());
        }
    };
}

}
