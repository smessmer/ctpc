#include "parsers/alpha.h"

using namespace ctpc;

namespace {

namespace test_alphasmall_failure {
    constexpr auto parsed = alpha_small()(Input{"ABC"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "ABC");
}
namespace test_alphasmall_success_a {
    constexpr auto parsed = alpha_small()(Input{"abc"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'a');
    static_assert(parsed.next().input == "bc");
}
namespace test_alphasmall_success_k {
    constexpr auto parsed = alpha_small()(Input{"kabc"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'k');
    static_assert(parsed.next().input == "abc");
}
namespace test_alphasmall_success_z {
    constexpr auto parsed = alpha_small()(Input{"zabc"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'z');
    static_assert(parsed.next().input == "abc");
}
namespace test_alphalarge_failure {
    constexpr auto parsed = alpha_large()(Input{"abc"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "abc");
}
namespace test_alphalarge_success_a {
    constexpr auto parsed = alpha_large()(Input{"ABC"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'A');
    static_assert(parsed.next().input == "BC");
}
namespace test_alphalarge_success_k {
    constexpr auto parsed = alpha_large()(Input{"KABC"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'K');
    static_assert(parsed.next().input == "ABC");
}
namespace test_alphalarge_success_z {
    constexpr auto parsed = alpha_large()(Input{"ZABC"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'Z');
    static_assert(parsed.next().input == "ABC");
}
namespace test_alphanumeric_failure_space {
    constexpr auto parsed = alphanumeric()(Input{" "});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == " ");
}
namespace test_alphanumeric_failure_newline {
    constexpr auto parsed = alphanumeric()(Input{"\n"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "\n");
}
namespace test_alphanumeric_success_a {
    constexpr auto parsed = alphanumeric()(Input{"a Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'a');
    static_assert(parsed.next().input == " Text");
}
namespace test_alphanumeric_success_k {
    constexpr auto parsed = alphanumeric()(Input{"k Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'k');
    static_assert(parsed.next().input == " Text");
}
namespace test_alphanumeric_success_z {
    constexpr auto parsed = alphanumeric()(Input{"z Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'z');
    static_assert(parsed.next().input == " Text");
}
namespace test_alphanumeric_success_A {
    constexpr auto parsed = alphanumeric()(Input{"A Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'A');
    static_assert(parsed.next().input == " Text");
}
namespace test_alphanumeric_success_K {
    constexpr auto parsed = alphanumeric()(Input{"K Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'K');
    static_assert(parsed.next().input == " Text");
}
namespace test_alphanumeric_success_Z {
    constexpr auto parsed = alphanumeric()(Input{"Z Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'Z');
    static_assert(parsed.next().input == " Text");
}
namespace test_alphanumeric_success_0 {
    constexpr auto parsed = alphanumeric()(Input{"0 Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == '0');
    static_assert(parsed.next().input == " Text");
}
namespace test_alphanumeric_success_5 {
    constexpr auto parsed = alphanumeric()(Input{"5 Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == '5');
    static_assert(parsed.next().input == " Text");
}
namespace test_alphanumeric_success_9 {
    constexpr auto parsed = alphanumeric()(Input{"9 Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == '9');
    static_assert(parsed.next().input == " Text");
}
namespace test_whitespace_failure {
    constexpr auto parsed = whitespace()(Input{"abc"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "abc");
}
namespace test_whitespace_success {
    constexpr auto parsed = whitespace()(Input{" abc"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == ' ');
    static_assert(parsed.next().input == "abc");
}
namespace test_whitespaces_empty {
    constexpr auto parsed = whitespaces()(Input{""});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "");
    static_assert(parsed.next().input == "");
}
namespace test_whitespaces_none {
    constexpr auto parsed = whitespaces()(Input{"abc"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "");
    static_assert(parsed.next().input == "abc");
}
namespace test_whitespaces_one {
    constexpr auto parsed = whitespaces()(Input{" abc"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == " ");
    static_assert(parsed.next().input == "abc");
}
namespace test_whitespaces_two {
    constexpr auto parsed = whitespaces()(Input{"  abc"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "  ");
    static_assert(parsed.next().input == "abc");
}
namespace test_whitespaces_five {
    constexpr auto parsed = whitespaces()(Input{"     abc"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "     ");
    static_assert(parsed.next().input == "abc");
}

}
