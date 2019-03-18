#include "funcsig_parser/identifier.h"

#include <gtest/gtest.h>

using namespace ctpc::funcsig_parser;

namespace {

namespace identifier_success {
    constexpr auto parsed = ctpc::funcsig_parser::identifier()(ctpc::Input{"my_identifier0345-andsomeinvalid stuff"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "my_identifier0345");
    static_assert(parsed.next().input == "-andsomeinvalid stuff");
}

namespace identifier_failure {
    constexpr auto parsed = ctpc::funcsig_parser::identifier()(ctpc::Input{"345_identifier_cant_start_with_numbers"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "345_identifier_cant_start_with_numbers");
}


}
