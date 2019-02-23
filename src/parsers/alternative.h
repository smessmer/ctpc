#pragma once

#include "parsers/parse_result.h"
#include "parsers/basic_parsers.h"

namespace ctpc {

namespace details {
    // TODO Seems the recursive template approach has quite bad codegen for >4 arguments. Figure out an alternative.

    template<class Result, class... Parsers> struct apply_alternative_parsers final {};

    template<class Result, class HeadParser, class... TailParsers>
    struct apply_alternative_parsers<Result, HeadParser, TailParsers...> final {
        static constexpr ParseResult<Result> call(Input input, ParseResult<Result>&& longest_failure, const HeadParser &headParser, const TailParsers &... tailParsers) {
            static_assert(std::is_convertible_v<parser_result_t<HeadParser>, Result>, "This shouldn't happen because Result is the common_type of all individual parser results");
            static_assert(std::is_same_v<Result, std::decay_t<Result>>);

            auto parsed = headParser(input);

            if (!parsed.is_failure()) {
                // SUCCESS and ERROR end the chain
                return ParseResult<Result>::unsafe_convert_from(std::move(parsed));
            }

            // The current parser failed. Try the next in the list.

            if (parsed.next().input.size() < longest_failure.next().input.size()) {
                // Current failure matched a longer prefix of the input than all previous parsers.
                // Remember this one as the result in case no parser returns success or error.
                return apply_alternative_parsers<Result, TailParsers...>::call(input, ParseResult<Result>::unsafe_convert_from(std::move(parsed)), tailParsers...);
            } else {
                // A previous parser had a longer match. Keep that.
                return apply_alternative_parsers<Result, TailParsers...>::call(input, std::move(longest_failure), tailParsers...);
            }
        }
    };

    template<class Result>
    struct apply_alternative_parsers<Result> final {
        static constexpr ParseResult<Result>&& call(Input /*input*/, ParseResult<Result>&& longest_failure) {
            return std::move(longest_failure);
        }
    };
}

template<class... Parsers>
constexpr auto alternative(Parsers&&... parsers) {
    using result_type = std::common_type_t<parser_result_t<Parsers>...>;
    return [parsers = std::make_tuple(std::forward<Parsers>(parsers)...)] (Input input) -> ParseResult<result_type> {
        return std::apply([input] (const auto&... parsers) {
            return details::apply_alternative_parsers<result_type, std::decay_t<Parsers>...>::call(input, ParseResult<result_type>::failure(input), parsers...);
        }, parsers);
    };
}
template<>
constexpr auto alternative() {
    return failure();
}

}
