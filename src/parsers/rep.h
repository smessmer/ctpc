#pragma once

#include "parsers/parse_result.h"
#include "parsers/basic_parsers.h"
#include "parsers/map.h"
#include "parsers/utils/cvector.h"

namespace ctpc {

// TODO Test this moves/copies elementParser/separatorParser correctly (i.e. works with movable-only and also count number of moves/copies)
template<size_t MAX_REPETITIONS = 1024, class ElementParser, class SeparatorParser>
constexpr auto repsep(ElementParser elementParser, SeparatorParser separatorParser) {
    using parse_result = ParseResult<cvector<parser_result_t<ElementParser>, MAX_REPETITIONS>>;
    return [elementParser = std::move(elementParser), separatorParser = std::move(separatorParser)] (Input input) -> parse_result {
        cvector<parser_result_t<ElementParser>, MAX_REPETITIONS> result;
        auto element_result = elementParser(input);
        while(element_result.is_success()) {
            result.push_back(element_result.result());
            input = element_result.next();

            auto separator_result = separatorParser(input);
            if (!separator_result.is_success()) {
                break;
            }
            element_result = elementParser(separator_result.next());
        }

        return parse_result::success(result, input);
    };
}

// TODO Test this moves parser correctly (i.e. works with movable-only and also count number of moves/copies)
template<size_t MAX_REPETITIONS = 1024, class Parser>
constexpr auto rep(Parser parser) {
    return repsep(std::move(parser), success());
}

// TODO Test this moves parser correctly (i.e. works with movable-only and also count number of moves/copies)
template<size_t MAX_REPETITIONS = 1024, class Parser>
constexpr auto rep1(Parser parser) {
    using parse_result = ParseResult<cvector<parser_result_t<Parser>, MAX_REPETITIONS>>;
    return flatMap(
            rep(std::move(parser)),
            [] (auto parsed) -> parse_result {
                if (!parsed.is_success()) {
                    return parsed;
                }
                if (parsed.result().size() == 0) {
                    return parse_result::failure(parsed.next());
                }
                return parsed;
            }
    );
}

}
