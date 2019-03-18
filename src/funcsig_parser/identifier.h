#pragma once

#include "parsers/rep.h"
#include "parsers/alternative.h"
#include "parsers/alpha.h"
#include "parsers/seq.h"
#include "parsers/match.h"

namespace ctpc {
namespace funcsig_parser {

namespace detail {
template<class Parser>
constexpr auto rep_count(Parser &&parser) {
    return rep(
        std::forward<Parser>(parser),
        []() -> int64_t { return 0; },
        [](int64_t *counter, const auto & /*result*/) {
            ++*counter;
        }
    );
}
}

constexpr auto identifier() {
    return match(
        seq(
            alternative(alpha(), elem('_')),
            detail::rep_count(alternative(alphanumeric(), elem('_')))
        )
    );
}

}
}
