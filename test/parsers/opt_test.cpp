#include "parsers/opt.h"
#include "parsers/string.h"
#include "testutils/move_helpers.h"
#include <gtest/gtest.h>

using namespace ctpc;

namespace {

namespace test_opt_success {
    constexpr auto parsed = opt(string("Found"))(Input{"FoundSomething"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().has_value() && parsed.result().value() == "Found");
    static_assert(parsed.next().input == "Something");
}
namespace test_opt_failure {
    constexpr auto parsed = opt(string("Found"))(Input{"FoSomethingElse"});
    static_assert(parsed.is_success());
    static_assert(!parsed.result().has_value());
    static_assert(parsed.next().input == "FoSomethingElse");
}

namespace test_opt_movableonly_success {
    constexpr auto parsed = opt(movable_only(string("Found")))(Input{"FoundSomething"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().has_value() && parsed.result().value() == "Found");
    static_assert(parsed.next().input == "Something");
}
namespace test_opt_movableonly_failure {
    constexpr auto parsed = opt(movable_only(string("Found")))(Input{"FoSomethingElse"});
    static_assert(parsed.is_success());
    static_assert(!parsed.result().has_value());
    static_assert(parsed.next().input == "FoSomethingElse");
}

TEST(OptParserTest, doesntCopyOrMoveMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveMoreThanAbsolutelyNecessary(
            [] (auto&& arg) {return opt(std::forward<decltype(arg)>(arg)); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");}
    );
}

}
