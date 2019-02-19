#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

namespace details {
    template<class... Results>
    struct seq_accumulator final {
        ParseResult<std::tuple<Results...>> results;
    };

    // TODO Test tuple_append
    // TODO Test forwarding
    template<class NextElem, class... PreviousElems, size_t... indices>
    constexpr std::tuple<PreviousElems..., NextElem> tuple_append(std::tuple<PreviousElems...>&& prev, NextElem&& next, std::index_sequence<indices...>) {
        return std::tuple<PreviousElems..., NextElem>{std::get<indices>(std::move(prev))..., std::forward<NextElem>(next)};
    }

    // TODO Test forwarding and stuff (i.e. how often is copy/move called) for operator<<.
    template<class NextParser, class... PreviousResults>
    constexpr seq_accumulator<PreviousResults..., parser_result_t<NextParser>> operator<<(
            seq_accumulator<PreviousResults...>&& previous_results, const NextParser& nextParser) {
        using result_type = ParseResult<std::tuple<PreviousResults..., parser_result_t<NextParser>>>;
        using result_accumulator = seq_accumulator<PreviousResults..., parser_result_t<NextParser>>;
        if (!previous_results.results.is_success()) {
            return result_accumulator{result_type::failure(previous_results.results.next())};
        } else {
            auto parsed = nextParser(previous_results.results.next());
            if (!parsed.is_success()) {
                return result_accumulator{result_type::failure(parsed.next())};
            } else {
                return result_accumulator{result_type::success(tuple_append(std::move(previous_results.results).result(), std::move(parsed).result(), std::make_index_sequence<sizeof...(PreviousResults)>()), parsed.next())};
            }
        }
    }
}

// TODO Test this (and other) parsers for movable-only result types (i.e. works with movable-only and also count number of moves/copies)
template<class... Parsers>
constexpr auto seq(Parsers... parsers) {
    using result_type = std::tuple<parser_result_t<Parsers>...>;
    // TODO Move parsers
    return [parsers...] (Input input) -> ParseResult<result_type> {
        return (details::seq_accumulator<>{ParseResult<std::tuple<>>::success(std::tuple<>(), input)} << ... << parsers).results;
    };
}

}
