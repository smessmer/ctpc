#pragma once

#include "parsers/parse_result.h"
#include "parsers/elem.h"
#include "parsers/alternative.h"
#include "parsers/integer.h"

namespace ctpc {

constexpr auto alpha_small() {
    return elem([] (char v) {
        return v >= 'a' && v <= 'z';
    });
}

constexpr auto alpha_large() {
    return elem([] (char v) {
        return v >= 'A' && v <= 'Z';
    });
}

constexpr auto alpha() {
    return alternative(alpha_small(), alpha_large());
}

constexpr auto alphanumeric() {
    return alternative(alpha_small(), alpha_large(), numeric());
}

constexpr auto whitespace() {
    return elem(' ');
}

/**
 * Match zero or more whitespace characters
 */
constexpr auto whitespaces() {
    return [] (Input input) {
        // find first non-whitespace character
        auto current = input.input.begin();
        while(current != input.input.end() && *current == ' ') {
            ++current;
        }

        Input next = Input{std::string_view(current, input.input.end() - current)};
        std::string_view match(input.input.begin(), current - input.input.begin());
        return ParseResult<std::string_view>::success(next, match);
    };
}

}
