#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

template<class Parser>
constexpr auto phrase(Parser&& parser) {
    return [parser = std::forward<Parser>(parser)] (Input input) -> ParseResult<parser_result_t<Parser>> {
        auto result = parser(input);
        if (result.is_success() && result.next().input.size() != 0) {
            result = ParseResult<parser_result_t<Parser>>::failure(result.next());
        }
        return result;
    };
}

}
