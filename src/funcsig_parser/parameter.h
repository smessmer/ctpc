#pragma once

#include "parsers/map.h"
#include "parsers/elem.h"
#include "funcsig_parser/identifier.h"

namespace ctpc {
namespace funcsig_parser {

struct Parameter final {
    std::string_view name;
    std::string_view type;
};

constexpr auto parameter() {
    return map(
        seq(identifier(), whitespaces(), elem(':'), whitespaces(), identifier()),
        [] (auto&& parsed) {
            return Parameter{std::get<0>(parsed), std::get<4>(parsed)};
        }
    );
}

}
}
