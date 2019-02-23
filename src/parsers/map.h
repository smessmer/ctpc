#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

template<class Parser, class MapFunction>
constexpr auto flatMap(Parser&& parser, MapFunction&& mapFunction) {
    using result_type = typename decltype(mapFunction(std::declval<ParseResult<parser_result_t<Parser>>>()))::result_type;
    return [parser = std::forward<Parser>(parser), mapFunction = std::forward<MapFunction>(mapFunction)] (Input input) -> ParseResult<result_type> {
        return mapFunction(parser(input));
    };
}

template<class Parser, class MapFunction>
constexpr auto map(Parser&& parser, MapFunction&& mapFunction) {
    // This could be implemented using flatMap() and a function wrapping the result, but that'd need
    // to move the mapFunction one more time. We take a different approach to avoid that move.
    using result_type = decltype(mapFunction(std::declval<parser_result_t<Parser>>()));
    return [parser = std::forward<Parser>(parser), mapFunction = std::forward<MapFunction>(mapFunction)] (Input input) -> ParseResult<result_type> {
        return parser(input).map(mapFunction);
    };
}

template<class Result, class Parser>
constexpr auto mapValue(Parser&& parser, Result&& value) {
    // This could be implemented using map() and a function always returning a constant, but that'd need
    // to move the value one more time. We take a different approach to avoid that move.
    return [parser = std::forward<Parser>(parser), value = std::forward<Result>(value)] (Input input) -> ParseResult<std::decay_t<Result>> {
        return parser(input).mapValue(value);
    };
}

}
