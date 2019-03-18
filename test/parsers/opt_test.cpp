#include "parsers/opt.h"
#include "parsers/string.h"
#include "testutils/move_helpers.h"
#include "testutils/error_parser.h"
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
namespace test_opt_error {
    constexpr auto parsed = opt(error_parser(string("Fo")))(Input{"FoSomethingElse"});
    static_assert(parsed.is_error());
    static_assert(parsed.next().input == "SomethingElse");
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
namespace test_opt_movableonly_error {
    constexpr auto parsed = opt(error_parser(movable_only(string("Fo"))))(Input{"FoSomethingElse"});
    static_assert(parsed.is_error());
    static_assert(parsed.next().input == "SomethingElse");
}

TEST(OptParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&& arg) {return opt(std::forward<decltype(arg)>(arg)); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");}
    );
}

namespace test_opt_works_with_movable_only_result_success {
    constexpr auto parsed = opt(success_parser_with_movableonly_result())(Input{"input"});
    static_assert(parsed.is_success());
}

namespace test_opt_works_with_movable_only_result_failure {
    constexpr auto parsed = opt(failure_parser_with_movableonly_result())(Input{"input"});
    static_assert(parsed.is_success());
}

namespace test_opt_works_with_movable_only_result_error {
    constexpr auto parsed = opt(error_parser_with_movableonly_result())(Input{"input"});
    static_assert(parsed.is_error());
}

TEST(OptParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary) {
    size_t copy_count = 0, move_count = 0;
    auto parsed = opt(success_parser_with_copycounting_result(&copy_count, &move_count))(Input{"input"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count);
    EXPECT_EQ(1, move_count);
}

}
