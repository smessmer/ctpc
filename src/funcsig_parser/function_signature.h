#pragma once

#include "funcsig_parser/parameter.h"
#include "parsers/rep.h"

namespace ctpc {
namespace funcsig_parser {

#define MAX_PARAMETERS 64

constexpr auto parameter_list(_compiletime_optimization optimization) {
    return repsep<MAX_PARAMETERS>(optimization, parameter(), seq(whitespaces(), elem(','), whitespaces()));
}

constexpr auto parameter_list(_runtime_optimization optimization) {
    return repsep(optimization, parameter(), seq(whitespaces(), elem(','), whitespaces()));
}

template<class Optimization>
struct FunctionSignature final {
    static_assert(std::is_same_v<_compiletime_optimization, Optimization> || std::is_same_v<_runtime_optimization, Optimization>);
    using ParameterContainer = std::conditional_t<
        std::is_same_v<_compiletime_optimization, Optimization>,
        cvector<Parameter, MAX_PARAMETERS>,
        std::vector<Parameter>>;

    std::string_view return_type;
    std::string_view name;
    ParameterContainer parameters;
};

template<class Optimization>
constexpr auto function_signature(Optimization optimization) {
    return map(
        seq(identifier(), whitespaces(), identifier(), whitespaces(), elem('('), whitespaces(), parameter_list(optimization), whitespaces(), elem(')')),
        [] (auto&& parsed) {
            return FunctionSignature<Optimization>{std::get<0>(parsed), std::get<2>(parsed), std::get<6>(parsed)};
        }
    );
}


}
}
