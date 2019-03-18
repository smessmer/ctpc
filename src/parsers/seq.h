#pragma once

#include "parsers/parse_result.h"

namespace ctpc {

namespace details {
    struct SeqResultStatus final {
        Input next;
        ResultStatus status;
    };

    template<class... Parsers> struct apply_seq_parsers final {};

    template<class HeadParser, class... TailParsers>
    struct apply_seq_parsers<HeadParser, TailParsers...> final {
        template<size_t current_index, class ResultBuffer>
        static constexpr SeqResultStatus call(Input input, ResultBuffer* result, const HeadParser& headParser, const TailParsers&... tailParsers) {
            static_assert(std::tuple_size_v<ResultBuffer> == current_index + 1 + sizeof...(TailParsers));
            static_assert(std::is_same_v<std::tuple_element_t<current_index, ResultBuffer>, ParseResult<parser_result_t<HeadParser>>>);

            auto& current_result = std::get<current_index>(*result);
            current_result = headParser(input);
            if (current_result.is_success()) {
                return apply_seq_parsers<TailParsers...>::template call<current_index + 1>(current_result.next(), result, tailParsers...);
            }

            return SeqResultStatus { current_result.next(), current_result.status() };
        }
    };

    template<>
    struct apply_seq_parsers<> final {
        template<size_t current_index, class ResultBuffer>
        static constexpr SeqResultStatus call(Input input, ResultBuffer* /*result*/) {
            static_assert(std::tuple_size_v<ResultBuffer> == current_index);

            return SeqResultStatus { input, ResultStatus::SUCCESS };
        }
    };

}

template<class... Parsers>
constexpr auto seq(Parsers&&... parsers) {
    using result_type = std::tuple<parser_result_t<Parsers>...>;
    return [parsers = std::make_tuple(std::forward<Parsers>(parsers)...)] (Input input) -> ParseResult<result_type> {
        return std::apply([input] (const auto&... parsers) {
            std::tuple<ParseResult<parser_result_t<Parsers>>...> result_buffer {ParseResult<parser_result_t<Parsers>>::failure(input)...};
            details::SeqResultStatus status = details::apply_seq_parsers<std::decay_t<Parsers>...>::template call<0>(
                    input, &result_buffer, parsers...
            );

            if (status.status == ResultStatus::SUCCESS) {
                return std::apply([&] (auto&&... results) {
                    ASSERT((... && results.is_success()));
                    return ParseResult<result_type>::success(status.next, std::move(results).result()...);
                }, std::move(result_buffer));
            }

            if (status.status == ResultStatus::FAILURE) {
                return ParseResult<result_type>::failure(status.next);
            }

            ASSERT (status.status == ResultStatus::ERROR);
            return ParseResult<result_type>::error(status.next);

        }, parsers);
    };
}

}
