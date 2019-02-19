#include "parsers/string.h"

using namespace ctpc;

namespace {

namespace test_string_success {
    constexpr auto parsed = string("Hello")(Input{"HelloAndSomeOtherText"});
    static_assert(parsed.is_success());
    static_assert(parsed.next().input == "AndSomeOtherText");
    static_assert(parsed.result() == "Hello");
}
namespace test_string_failure_immediate {
    constexpr auto parsed = string("Hello")(Input{"AndSomeOtherText"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "AndSomeOtherText");
}
namespace test_string_failure_later {
    constexpr auto parsed = string("Hello")(Input{"HellAndSomeOtherText"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "AndSomeOtherText");
}

}
