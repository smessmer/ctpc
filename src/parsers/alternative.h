#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

namespace details {
    template<class Result>
    struct alternative_accumulator final {
        Input initialInput;

        // while trying failing parsers from left to right, this remembers the failure with the longest match.
        // if a parser returns a success or fatal error, that result will be remembered instead, and
        // no more parsers will be tried.
        ParseResult<Result> firstSuccessOrErrorOrFailureWithLongestMatch;
    };

    template<class Result, class Parser>
    constexpr alternative_accumulator<Result> operator<<(
            alternative_accumulator<Result>&& previous_result, Parser nextParser) {
        static_assert(std::is_convertible_v<parser_result_t<Parser>, Result>, "This shouldn't happen bebcause Result is the common_type of all individual parser results");
        if (previous_result.firstSuccessOrErrorOrFailureWithLongestMatch.is_failure()) {
            auto parsed = nextParser(previous_result.initialInput);
            if (!parsed.is_failure() || parsed.next().input.size() < previous_result.firstSuccessOrErrorOrFailureWithLongestMatch.next().input.size()) {
                // the current parser was either successful, threw a fatal error, or was a failure with a longer match.
                // we want to store it in firstSuccessOrErrorOrFailureWithLongestMatch.
                return alternative_accumulator<Result>{previous_result.initialInput, std::move(parsed)};
            } else {
                // the current parser was a failure but with a shorter match.
                // we want to keep firstSuccessOrErrorOrFailureWithLongestMatch from the previous, longer match
                return previous_result;
            }
        } else {
            // previous_result was SUCCESS or ERROR
            return previous_result;
        }
    }
}

// TODO Test this moves correctly (i.e. works with movable-only and also count number of moves/copies)
template<class... Parsers>
constexpr auto alternative(Parsers... parsers) {
    using result_type = std::common_type_t<parser_result_t<Parsers>...>;
    // TODO We should move in the parsers
    return [parsers...] (Input input) -> ParseResult<result_type> {
        return (details::alternative_accumulator<result_type>{input, ParseResult<result_type>::failure(input)} << ... << parsers).firstSuccessOrErrorOrFailureWithLongestMatch;
    };
}
template<>
constexpr auto alternative() {
    return [] (Input input) -> ParseResult<std::nullptr_t> {
        return ParseResult<std::nullptr_t>::failure(input);
    };
}

}
