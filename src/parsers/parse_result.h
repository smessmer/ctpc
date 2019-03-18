#pragma once

#include <string_view>
#include <optional>
#include "parsers/utils/assert.h"

namespace ctpc {

// TODO Add test cases for ParseResult
// TODO Add error messages
// TODO For all parsers: Test mutable only results, and also count copying&moving of the result


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
class ParseResult final {
public:
    using result_type = T;

    template<class... _T>
    static constexpr ParseResult<T> success(Input next, _T&&... result) {
        static_assert(std::is_constructible_v<T, std::decay_t<_T>...>, "Invalid argument type");
        ParseResult<T> created(next, std::forward<_T>(result)...);
        ASSERT(created.status_ == ResultStatus::SUCCESS);
        return created;
    }

    static constexpr ParseResult failure(Input next) {
        return ParseResult(next, ResultStatus::FAILURE);
    }

    static constexpr ParseResult error(Input next) {
        return ParseResult(next, ResultStatus::ERROR);
    }

    // Warning: This function might return an rvalue reference, pointing back to the potentially dead argument.
    // Please make sure you store the result to a value before the argument gets destructed.
    // If you're unsure, better use the safer convert_from() instead of unsafe_convert_from(),
    // which always returns a new value.
    template<class U> static constexpr decltype(auto) unsafe_convert_from(ParseResult<U>&& source) {
        static_assert(std::is_convertible_v<U, T>, "Invalid argument");

        if constexpr(std::is_same_v<U, T>) {
            // note: Even if the U==T, the else clause below would work, but it would incur an additional move constructor
            // call, which we can avoid in the then-clause here by directly returning the rvalue reference from the input.
            return std::move(source);
        } else {
            if (source.result_.has_value()) {
                ASSERT(source.status_ == ResultStatus::SUCCESS);
                return ParseResult(source.next_, std::move(*source.result_));
            } else {
                return ParseResult(source.next_, source.status_);
            }
        }
    }

    template<class U> static constexpr ParseResult convert_from(ParseResult<U>&& source) {
        // source lives at least until after this function created its return value,
        // which will convert any potential rvalue into a value and makes this safe.
        return unsafe_convert_from(std::move(source));
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

    constexpr T& result() & {
        ASSERT(status_ == ResultStatus::SUCCESS);
        ASSERT(result_.has_value());
        return *result_;
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

    constexpr ResultStatus status() const {
        return status_;
    }

    constexpr Input next() const {
        return next_;
    }

    template<class MapFunction>
    constexpr auto map(MapFunction&& mapper) && {
        using result_type = decltype(std::forward<MapFunction>(mapper)(std::move(*result_)));
        switch(status_) {
            case ResultStatus::SUCCESS: return ParseResult<result_type>::success(next_, std::forward<MapFunction>(mapper)(std::move(*result_)));
            case ResultStatus::FAILURE: return ParseResult<result_type>::failure(next_);
            case ResultStatus::ERROR: return ParseResult<result_type>::error(next_);
        }
    }

    template<class U>
    constexpr auto mapValue(U&& newValue) {
        using result_type = std::decay_t<U>;
        switch(status_) {
            case ResultStatus::SUCCESS: return ParseResult<result_type>::success(next_, std::forward<U>(newValue));
            case ResultStatus::FAILURE: return ParseResult<result_type>::failure(next_);
            case ResultStatus::ERROR: return ParseResult<result_type>::error(next_);
        }
    }

    constexpr void setNext(Input next) {
        next_ = std::move(next);
    }

private:
    template<class... _T>
    constexpr ParseResult(Input next, _T&&... result)
            : result_(std::in_place, std::forward<_T>(result)...), next_(std::move(next)), status_(ResultStatus::SUCCESS) {
        // note: the std::in_place above is important so that if T == optional<...> and _T == std::nullopt_t,
        //       it actually initializes result_ as an optional *with* a value of std::nullopt_t and not without a value.
        ASSERT(status_ == ResultStatus::SUCCESS);
        ASSERT(result_.has_value());
    }

    constexpr ParseResult(Input next, ResultStatus status)
            : result_(std::nullopt), next_(std::move(next)), status_(std::move(status)) {
        ASSERT(status_ != ResultStatus::SUCCESS);
    }

    template<class U> friend class ParseResult;

private:
    std::optional<T> result_;
    Input next_;
    ResultStatus status_;
};


// TODO Test parser_result_t and is_parser
template<class Parser>
using parser_result_t = typename decltype(std::declval<Parser>()(std::declval<Input>()))::result_type;

template<class Parser, class Enable = void> struct is_parser : std::false_type {};
template<class Parser>
struct is_parser<Parser, std::enable_if_t<!std::is_same_v<void, parser_result_t<Parser>>>> : std::true_type {};

template<class Parser> using is_parser_t = typename is_parser<Parser>::type;
template<class Parser> constexpr inline bool is_parser_v = is_parser<Parser>::value;

}
