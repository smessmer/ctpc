#include "parsers/match.h"
#include "parsers/integer.h"
#include "testutils/move_helpers.h"
#include "testutils/error_parser.h"
#include <gtest/gtest.h>

using namespace ctpc;

namespace {

    namespace test_match_success {
        constexpr auto parsed = match(integer())(Input{"23473Something"});
        static_assert(parsed.is_success());
        static_assert(parsed.result() == "23473");
        static_assert(parsed.next().input == "Something");
    }
    namespace test_match_failure {
        constexpr auto parsed = match(integer())(Input{"Something"});
        static_assert(parsed.is_failure());
        static_assert(parsed.next().input == "Something");
    }
    namespace test_match_error {
        constexpr auto parsed = match(error())(Input{"Something"});
        static_assert(parsed.is_error());
        static_assert(parsed.next().input == "Something");
    }

    namespace test_match_movableonly_success {
        constexpr auto parsed = match(movable_only(integer()))(Input{"23473Something"});
        static_assert(parsed.is_success());
        static_assert(parsed.result() == "23473");
        static_assert(parsed.next().input == "Something");
    }
    namespace test_match_movableonly_failure {
        constexpr auto parsed = match(movable_only(integer()))(Input{"Something"});
        static_assert(parsed.is_failure());
        static_assert(parsed.next().input == "Something");
    }
    namespace test_match_movableonly_error {
        constexpr auto parsed = match(movable_only(error()))(Input{"Something"});
        static_assert(parsed.is_error());
        static_assert(parsed.next().input == "Something");
    }

    TEST(MatchParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary) {
        testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
                [] (auto&& arg) {return match(std::forward<decltype(arg)>(arg)); },
                {ctpc::Input{""}, ctpc::Input{"2343Bla"}, ctpc::Input{"NotFound"}},
                [] () {return integer();}
        );
    }

    namespace test_match_works_with_movable_only_result_success {
        constexpr auto parsed = match(success_parser_with_movableonly_result())(Input{"input"});
        static_assert(parsed.is_success());
    }

    namespace test_match_works_with_movable_only_result_failure {
        constexpr auto parsed = match(failure_parser_with_movableonly_result())(Input{"input"});
        static_assert(parsed.is_failure());
    }

    namespace test_match_works_with_movable_only_result_error {
        constexpr auto parsed = match(error_parser_with_movableonly_result())(Input{"input"});
        static_assert(parsed.is_error());
    }

    TEST(MatchParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary) {
        size_t copy_count = 0, move_count = 0;
        auto parsed = match(success_parser_with_copycounting_result(&copy_count, &move_count))(Input{"input"});
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(0, copy_count);
        EXPECT_EQ(0, move_count);
    }

}
