#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

// TODO Test that this moves/copies parser/mapFunction correctly (i.e. works with movable-only and also count number of moves/copies)
template<class Parser, class MapFunction>
constexpr auto flatMap(Parser parser, MapFunction mapFunction) {
    using result_type = typename decltype(std::declval<MapFunction>()(std::declval<ParseResult<parser_result_t<Parser>>>()))::result_type;
    return [parser = std::move(parser), mapFunction = std::move(mapFunction)] (Input input) -> ParseResult<result_type> {
        auto result = parser(input);
        return mapFunction(result);
    };
}

// TODO Here (and other parsers) test movable only mapFunction (i.e. works with movable-only and also count number of moves/copies)
template<class Parser, class MapFunction>
constexpr auto map(Parser parser, MapFunction mapFunction) {
    using result_type = decltype(std::declval<MapFunction>()(std::declval<parser_result_t<Parser>>()));
    return flatMap(std::move(parser), [mapFunction = std::move(mapFunction)] (auto parsed) -> ParseResult<result_type> {
        if (parsed.is_success()) {
            return ParseResult<result_type>::success(mapFunction(parsed.result()), parsed.next());
        } else {
            return ParseResult<result_type>::failure(parsed.next());
        }
    });
}

// TODO Here (and other parsers) test movable only parsers (i.e. works with movable-only and also count number of moves/copies)
// TODO Here (and other parsers) test movable only result (i.e. works with movable-only and also count number of moves/copies)
template<class Result, class Parser>
constexpr auto mapValue(Parser parser, Result value) {
    return map(std::move(parser), [value = std::move(value)] (auto) {return value;});
}

}
