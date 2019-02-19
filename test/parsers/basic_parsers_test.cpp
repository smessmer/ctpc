#include "parsers/basic_parsers.h"

using namespace ctpc;

namespace {

namespace test_success {
    constexpr auto parsed = success()(Input{"sometext"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == nullptr);
    static_assert(parsed.next().input == "sometext");
}
namespace test_failure {
    constexpr auto parsed = failure()(Input{"sometext"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "sometext");
}
namespace test_error {
    constexpr auto parsed = error()(Input{"sometext"});
    static_assert(parsed.is_error());
    static_assert(parsed.next().input == "sometext");
}

}
