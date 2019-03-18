#pragma once

#include "parsers/parse_result.h"
#include "parsers/map.h"
#include "parsers/alternative.h"

template<class Parser>
constexpr auto error_parser(Parser&& error_case) {
    return ctpc::flatMap(std::forward<Parser>(error_case), [] (auto result) {
        if (result.is_success()) {
            return ctpc::ParseResult<ctpc::parser_result_t<Parser>>::error(result.next());
        } else {
            return result;
        }
    });
}

template<class ErrorCaseParser, class SuccessCaseParser>
constexpr auto error_parser(ErrorCaseParser&& error_case, SuccessCaseParser&& success_case) {
    return ctpc::alternative(
        std::forward<SuccessCaseParser>(success_case),
        error_parser(std::forward<ErrorCaseParser>(error_case))
    );
}
