#pragma once

#include <string_view>
#include <optional>
#include "parsers/utils/assert.h"

namespace ctpc {

// TODO Add test cases for ParseResult
// TODO Add test cases to all combined parsers for how they handle ERROR state
// TODO Add error messages


// TODO templatize char away?
struct Input final {
    std::string_view input;
};

/*
 * SUCCESS: Parser successfully parsed the input
 * FAILURE: Parser didn't successfully parse the input, but alternative parsers can be tried (if chained as in the regex '|' operator)
 * ERROR: Parser hit a fatal error. Abort parsing and don't try alternatives.
 */
enum class ResultStatus : uint8_t {SUCCESS, FAILURE, ERROR};

template<class T>
struct ParseResult final {
public:
    using result_type = T;

    static constexpr ParseResult success(T result, Input next) {
        return ParseResult(result, next, ResultStatus::SUCCESS);
    }

    static constexpr ParseResult failure(Input next) {
        return ParseResult(std::nullopt, next, ResultStatus::FAILURE);
    }

    static constexpr ParseResult error(Input next) {
        return ParseResult(std::nullopt, next, ResultStatus::ERROR);
    }

    constexpr const T& result() const & {
        ASSERT(status_ == ResultStatus::SUCCESS);
        ASSERT(result_.has_value());
        return *result_;
    }

    constexpr T&& result() && {
        ASSERT(status_ == ResultStatus::SUCCESS);
        ASSERT(result_.has_value());
        return *std::move(result_);
    }

    constexpr bool is_success() const {
        return status_ == ResultStatus::SUCCESS;
    }

    constexpr bool is_failure() const {
        return status_ == ResultStatus::FAILURE;
    }

    constexpr bool is_error() const {
        return status_ == ResultStatus::ERROR;
    }

    constexpr Input next() const {
        return next_;
    }

private:
    constexpr ParseResult(std::optional<T> result, Input next, ResultStatus status)
            : result_(std::move(result)), next_(std::move(next)), status_(std::move(status)) {}

    std::optional<T> result_;
    Input next_;
    ResultStatus status_;
};

template<class Parser>
using parser_result_t = typename decltype(std::declval<Parser>()(std::declval<Input>()))::result_type;

}
