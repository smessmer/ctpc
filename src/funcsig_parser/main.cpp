#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <optional>
#include <array>
#include <cmath>
#include "parsers/utils/cvector.h"
#include "parsers/utils/assert.h"
#include "parsers/basic_parsers.h"
#include "parsers/alternative.h"
#include "parsers/phrase.h"
#include "parsers/string.h"
#include "parsers/opt.h"
#include "parsers/map.h"
#include "parsers/elem.h"
#include "parsers/integer.h"
#include "parsers/seq.h"
#include "parsers/alpha.h"
#include "parsers/rep.h"

using namespace ctpc;

struct Parameter final {
    std::string_view name;
    std::string_view type;
};

template<class Parser>
constexpr auto rep_count(Parser&& parser) {
    return rep(
        std::forward<Parser>(parser),
        [] () -> int64_t {return 0;},
        [] (int64_t* counter, const auto& /*result*/) {
            ++*counter;
        }
    );
}

constexpr auto identifier() {
    return [] (Input input) {
        return map (
                seq(
                    alternative(alpha(), elem('_')),
                    rep_count(alternative(alphanumeric(), elem('_')))
                ),
                [input] (const auto& parsed) {
                    size_t num_chars_parsed = 1 + std::get<1>(parsed);
                    return input.input.substr(0, num_chars_parsed);
                }
        )(input);
    };
}

namespace identifier_success {
    constexpr auto parsed = identifier()(Input{"my_identifier0-andsomeinvalid stuff"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "my_identifier0");
    static_assert(parsed.next().input == "-andsomeinvalid stuff");
}

constexpr auto opt_whitespaces() {
    return [] (Input input) {
        return map(
            rep_count(whitespace()),
            [input] (const auto& parsed) {
                size_t num_chars_parsed = parsed;
                return input.input.substr(0, num_chars_parsed);
            }
        )(input);
    };
}

constexpr auto parameter() {
    return map(
            seq(identifier(), opt_whitespaces(), elem(':'), opt_whitespaces(), identifier()),
            [] (auto parsed) {
                return Parameter{std::get<0>(parsed), std::get<4>(parsed)};
            }
    );
}

#define MAX_PARAMETERS 64

constexpr auto parameter_list() {
    return repsep<cvector<Parameter, MAX_PARAMETERS>>(parameter(), seq(opt_whitespaces(), elem(','), opt_whitespaces()));
}

struct FunctionSignature final {
    std::string_view return_type;
    std::string_view name;
    cvector<Parameter, MAX_PARAMETERS> parameters;
};

constexpr auto function_signature() {
    return map(
            seq(identifier(), opt_whitespaces(), identifier(), opt_whitespaces(), elem('('), opt_whitespaces(), parameter_list(), opt_whitespaces(), elem(')')),
            [] (auto parsed) {
                return FunctionSignature{std::get<0>(parsed), std::get<2>(parsed), std::get<6>(parsed)};
            }
    );
}

namespace function_signature_no_params {
    constexpr auto parsed = phrase(function_signature())(Input{"Int some_other_func()"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().return_type == "Int");
    static_assert(parsed.result().name == "some_other_func");
    static_assert(parsed.result().parameters.size() == 0);
}
namespace function_signature_no_params_whitespaces {
    constexpr auto parsed = phrase(function_signature())(Input{"Int some_other_func ( )"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().return_type == "Int");
    static_assert(parsed.result().name == "some_other_func");
    static_assert(parsed.result().parameters.size() == 0);
}
namespace function_signature_test {
    constexpr auto parsed = phrase(function_signature())(Input{"void my_func(arg1: Int, arg2: String)"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().return_type == "void");
    static_assert(parsed.result().name == "my_func");
    static_assert(parsed.result().parameters.size() == 2);
    static_assert(parsed.result().parameters[0].name == "arg1");
    static_assert(parsed.result().parameters[0].type == "Int");
    static_assert(parsed.result().parameters[1].name == "arg2");
    static_assert(parsed.result().parameters[1].type == "String");
}
namespace function_signature_whitespaces {
    constexpr auto parsed = phrase(function_signature())(Input{"Int my_other_func ( arg1 : String , arg2 : Double )"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().return_type == "Int");
    static_assert(parsed.result().name == "my_other_func");
    static_assert(parsed.result().parameters.size() == 2);
    static_assert(parsed.result().parameters[0].name == "arg1");
    static_assert(parsed.result().parameters[0].type == "String");
    static_assert(parsed.result().parameters[1].name == "arg2");
    static_assert(parsed.result().parameters[1].type == "Double");
}



















#include <tuple>
#include <string>
#include <cstring>

template<class TYPE> struct TypeDef final {
    std::string_view name;

    using type = TYPE;
};

constexpr auto types = std::make_tuple(
        TypeDef<const char*>{"String"},
        TypeDef<int>{"Int"}
);

template<class Tuple, class Condition, size_t current = 0>
constexpr size_t find_if(const Tuple& tuple, Condition&& cond) {
    if constexpr (current >= std::tuple_size<Tuple>::value) {
        throw std::logic_error("not found");
    } else {
        if (std::forward<Condition>(cond)(std::get<current>(tuple))) {
            return current;
        } else {
            return find_if<Tuple, Condition, current + 1>(tuple, std::forward<Condition>(cond));
        }
    }
}

constexpr size_t from_name(std::string_view name) {
    return find_if(types, [name] (const auto& type) {
        return type.name == name;
    });
}

template<size_t i> using gen_type = typename std::tuple_element<i, decltype(types)>::type::type;

void main1() {
    gen_type<from_name("Int")> a = 3;
    gen_type<from_name("String")> b = "my string";
}


constexpr uint64_t from_signature_old(std::string_view signature_string) {
    auto parsed = phrase(function_signature())(Input{signature_string});
    if (!parsed.is_success()) {
        throw std::logic_error("Error parsing schema"); // TODO error message
    }
    auto signature = parsed.result();
    uint64_t result = 1; // start with 1 not zero so that we don't delete leading zeroes and get the wrong number of types
    for (const auto& parameter : signature.parameters) {
        if ((std::numeric_limits<uint64_t>::max() - from_name(parameter.type)) / std::tuple_size_v<decltype(types)> < result) {
            throw std::logic_error("out of uint64_t range");
        }
        result *= std::tuple_size_v<decltype(types)>;
        result += from_name(parameter.type);
    }
    if ((std::numeric_limits<uint64_t>::max() - from_name(signature.return_type)) / std::tuple_size_v<decltype(types)> < result) {
        throw std::logic_error("out of uint64_t range");
    }
    result *= std::tuple_size_v<decltype(types)>;
    result += from_name(signature.return_type);
    return result;
}

namespace detail{
    template<class Tuple1, class Tuple2> struct tuple_concat final {};
    template<class... Types1, class... Types2>
    struct tuple_concat<std::tuple<Types1...>, std::tuple<Types2...>> final {
    using type = std::tuple<Types1..., Types2...>;
};
template<class Tuple1, class Tuple2> using tuple_concat_t = typename tuple_concat<Tuple1, Tuple2>::type;

template<uint64_t typeids> struct types_from_ids final {
    using type = tuple_concat_t<
            typename types_from_ids<typeids / std::tuple_size_v<decltype(types)>>::type,
    std::tuple<gen_type<typeids % std::tuple_size_v<decltype(types)>>>
    >;
};
template<> struct types_from_ids<1> {
    using type = std::tuple<>;
};
template<uint64_t typeids> using types_from_ids_t = typename types_from_ids<typeids>::type;

template<class return_type, class parameter_types> struct gen_signature final {};
template<class return_type, class... parameter_types>
struct gen_signature<return_type, std::tuple<parameter_types...>> final {
using type = return_type(parameter_types...);
};
template<class return_type, class parameter_types>
using gen_signature_t = typename gen_signature<return_type, parameter_types>::type;
}

template<uint64_t schema> class gen_schema {
private:
    static constexpr size_t base = std::tuple_size_v<decltype(types)>;
public:
    static constexpr size_t num_parameters = std::log(schema) / std::log(base) - 1; // TODO log not constexpr in actual standard or clang
    using return_type = gen_type<schema % base>;
public:
// TODO use types_from_ids_t also to get return type, don't do manual calculations in here
    using parameter_types = ::detail::types_from_ids_t<schema / base>;

    using signature = ::detail::gen_signature_t<return_type, parameter_types>;
};
/*
template<size_t result_type, size_t... arg_types> class gen_schema {
public:
static constexpr size_t num_parameters = sizeof...(arg_types);
using return_type = gen_type<result_type>;
using parameter_types = std::tuple<gen_type<arg_types>...>;

using signature = ::detail::gen_signature_t<return_type, parameter_types>;
};*/

namespace test_schema_0parameters_int {
    using result = gen_schema<from_signature_old("Int myfunc()")>;
    static_assert(std::is_same_v<int(), result::signature>);
}
namespace test_schema_0parameters_string {
    using result = gen_schema<from_signature_old("String myfunc()")>;
    static_assert(std::is_same_v<const char* (), result::signature>);
}
namespace test_schema_1parameter_intint {
    using result = gen_schema<from_signature_old("Int myfunc(arg1: Int)")>;
    static_assert(std::is_same_v<int(int), result::signature>);
}
namespace test_schema_1parameter_intstring {
    using result = gen_schema<from_signature_old("Int myfunc(arg1: String)")>;
    static_assert(std::is_same_v<int(const char*), result::signature>);
}
namespace test_schema_1parameter_stringint {
    using result = gen_schema<from_signature_old("String myfunc(arg1: Int)")>;
    static_assert(std::is_same_v<const char* (int), result::signature>);
}
namespace test_schema_1parameter_stringstring {
    using result = gen_schema<from_signature_old("String myfunc(arg1: String)")>;
    static_assert(std::is_same_v<const char* (const char*), result::signature>);
}
namespace test_schema_3parameter_stringstringint {
    using result = gen_schema<from_signature_old("String myfunc(arg1: String, arg2: Int)")>;
    static_assert(std::is_same_v<const char* (const char*, int), result::signature>);
}
namespace test_schema_3parameter_stringintstring {
    using result = gen_schema<from_signature_old("String myfunc(arg1: Int, arg2: String)")>;
    static_assert(std::is_same_v<const char* (int, const char*), result::signature>);
}
namespace test_schema_3parameter_intstringstring {
    using result = gen_schema<from_signature_old("Int myfunc(arg1: String, arg2: String)")>;
    static_assert(std::is_same_v<int (const char*, const char*), result::signature>);
}










template<size_t index>
constexpr int64_t param_type(const FunctionSignature& schema) {
    if (schema.parameters.size() > index) {
        return from_name(schema.parameters[index].type);
    } else {
        return -1;
    }
}

template<class Type, class Tuple>
struct tuple_prepend final {};
template<class Type, class... TupleTypes>
struct tuple_prepend<Type, std::tuple<TupleTypes...>> final {
using type = std::tuple<Type, TupleTypes...>;
};
template<class Type, class Tuple>
using tuple_prepend_t = typename tuple_prepend<Type, Tuple>::type;

template<typename Enable, int64_t... type_ids>
struct gen_type_tuple final {};
template<>
struct gen_type_tuple<void> final {
    using type = std::tuple<>;
};
template<int64_t head, int64_t... tail>
struct gen_type_tuple<std::enable_if_t<head == -1>, head, tail...> final {
using type = typename gen_type_tuple<void, tail...>::type;
};
template<int64_t head, int64_t... tail>
struct gen_type_tuple<std::enable_if_t<head != -1>, head, tail...> final {
using head_type = gen_type<head>;
using type = tuple_prepend_t<head_type, typename gen_type_tuple<void, tail...>::type>;
};
template<int64_t... type_ids> using gen_type_tuple_t = typename gen_type_tuple<void, type_ids...>::type;

template<class result_type, class parameter_type_tuple>
struct gen_function_type_ final {};
template<class result_type, class... parameter_types>
struct gen_function_type_<result_type, std::tuple<parameter_types...>> final {
using type = result_type (parameter_types...);
};

template<int64_t result_type_id, int64_t... param_type_ids>
struct gen_function_type final {
private:
    using result_type = gen_type<result_type_id>;
    static constexpr auto valid_param_indices = std::make_tuple((param_type_ids != -1)...);
//static constexpr uint64_t num_parameters = ((param_type_ids != -1) + ...);
    using param_types = gen_type_tuple_t<param_type_ids...>;
public:
    using type = typename gen_function_type_<result_type, param_types>::type;
};
template<int64_t result_type_id, int64_t... param_type_ids>
using gen_function_type_t = typename gen_function_type<result_type_id, param_type_ids...>::type;

#include <boost/preprocessor/repetition/repeat.hpp>

#define __PARAM_TYPE__(z, n, text) param_type<n>(parsed),
#define GEN_FUNCTION_TYPE(parsed) gen_function_type_t< \
from_name(parsed.return_type), \
BOOST_PP_REPEAT(MAX_PARAMETERS, __PARAM_TYPE__, void) \
-1 /* end value because BOOST_PP_REPEAT ends with a comma */ \
>

constexpr auto parse_function_signature(std::string_view input) {
    auto result = phrase(function_signature())(Input{"Int myfunc()"});
    if (!result.is_success()) {
        throw std::logic_error("Couldn't parse function signature");
    }
    return result.result();
}

namespace test_schema_0parameters_int {
    constexpr auto parsed = parse_function_signature("Int myfunc()");
    using function = GEN_FUNCTION_TYPE(parsed);
    static_assert(std::is_same_v<int(), result::signature>);
}
namespace test_schema_0parameters_string {
    constexpr auto parsed = parse_function_signature("String myfunc()");
    using function = GEN_FUNCTION_TYPE(parsed);
    static_assert(std::is_same_v<const char* (), result::signature>);
}
namespace test_schema_1parameter_intint {
    constexpr auto parsed = parse_function_signature("Int myfunc(arg1: Int)");
    using function = GEN_FUNCTION_TYPE(parsed);
    static_assert(std::is_same_v<int(int), result::signature>);
}
namespace test_schema_1parameter_intstring {
    constexpr auto parsed = parse_function_signature("Int myfunc(arg1: String)");
    using function = GEN_FUNCTION_TYPE(parsed);
    static_assert(std::is_same_v<int(const char*), result::signature>);
}
namespace test_schema_1parameter_stringint {
    constexpr auto parsed = parse_function_signature("String myfunc(arg1: Int)");
    using function = GEN_FUNCTION_TYPE(parsed);
    static_assert(std::is_same_v<const char* (int), result::signature>);
}
namespace test_schema_1parameter_stringstring {
    constexpr auto parsed = parse_function_signature("String myfunc(arg1: String)");
    using function = GEN_FUNCTION_TYPE(parsed);
    static_assert(std::is_same_v<const char* (const char*), result::signature>);
}
namespace test_schema_3parameter_stringstringint {
    constexpr auto parsed = parse_function_signature("String myfunc(arg1: String, arg2: Int)");
    using function = GEN_FUNCTION_TYPE(parsed);
    static_assert(std::is_same_v<const char* (const char*, int), result::signature>);
}
namespace test_schema_3parameter_stringintstring {
    constexpr auto parsed = parse_function_signature("String myfunc(arg1: Int, arg2: String)");
    using function = GEN_FUNCTION_TYPE(parsed);
    static_assert(std::is_same_v<const char* (int, const char*), result::signature>);
}
namespace test_schema_3parameter_intstringstring {
    constexpr auto parsed = parse_function_signature("Int myfunc(arg1: String, arg2: String)");
    using function = GEN_FUNCTION_TYPE(parsed);
    static_assert(std::is_same_v<int (const char*, const char*), result::signature>);
}


int main() {
    constexpr auto parsed = phrase(repsep<cvector<std::string_view, 1024>>(identifier(), whitespace()))(Input{"first second third fourth"});
    static_assert(parsed.is_success(), "");
    constexpr auto result = parsed.result();
    std::vector<std::string_view> runtime_value = cvector_to_vector<result.size()>(result);
    for(const auto& i : runtime_value) {
        std::cout << i << std::endl;
    }
}
