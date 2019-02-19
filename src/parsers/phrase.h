#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

template<class Parser>
constexpr auto phrase(Parser parser) {
    // TODO Test that this moves/copies the Parser correctly (i.e. works with movable-only and also count number of moves/copies)
    return [parser = std::move(parser)] (Input input) -> ParseResult<parser_result_t<Parser>> {
        auto result = parser(input);
        if (result.is_success() && result.next().input.size() == 0) {
            return result;
        } else {
            return ParseResult<parser_result_t<Parser>>::failure(result.next());
        }
    };
}

}
