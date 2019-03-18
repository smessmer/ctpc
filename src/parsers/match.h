#pragma once

#include "parsers/map.h"

namespace ctpc {

/**
 * This parser wraps another parser and discards its result.
 * Instead, if the wrapped parser was successful, it returns a std::string_view
 * containing the part of the input sequence matched by the inner parser.
 */
template<class Parser>
constexpr auto match(Parser&& parser) {
    return [parser = std::forward<Parser>(parser)] (Input input) {
        auto parsed = parser(input);

        if (parsed.is_success()) {
            const int64_t match_length = input.input.size() - parsed.next().input.size();
            return ParseResult<std::string_view>::success(parsed.next(), input.input.substr(0, match_length));
        }
        if (parsed.is_failure()) {
            return ParseResult<std::string_view>::failure(parsed.next());
        }
        ASSERT(parsed.is_error());
        return ParseResult<std::string_view>::error(parsed.next());
    };
}

}
