#include "parsers/phrase.h"
#include "parsers/string.h"
#include "testutils/move_helpers.h"
#include <gtest/gtest.h>

using namespace ctpc;

namespace {

namespace test_phrase_success {
    constexpr auto parsed = phrase(string("Hello"))(Input{"Hello"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "Hello");
    static_assert(parsed.next().input == "");
}
namespace test_phrase_failure_in_inner_parser {
    constexpr auto parsed = phrase(string("Hello"))(Input{"HellAnd"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "And");
}
namespace test_phrase_failure_in_phrase {
    constexpr auto parsed = phrase(string("Hello"))(Input{"HelloAnd"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "And");
}

namespace test_phrase_movableonly_success {
    constexpr auto parsed = phrase(movable_only(string("Hello")))(Input{"Hello"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "Hello");
    static_assert(parsed.next().input == "");
}
namespace test_phrase_movableonly_failure_in_inner_parser {
    constexpr auto parsed = phrase(movable_only(string("Hello")))(Input{"HellAnd"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "And");
}
namespace test_phrase_movableonly_failure_in_phrase {
    constexpr auto parsed = phrase(movable_only(string("Hello")))(Input{"HelloAnd"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "And");
}

TEST(PhraseParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&& arg) {return phrase(std::forward<decltype(arg)>(arg)); },
            {ctpc::Input{""}, ctpc::Input{"Found"}, ctpc::Input{"SomethingElse"}, ctpc::Input{"FoundAndSomeMore"}},
            [] () {return string("Found");}
    );
}

namespace test_phrase_works_with_movable_only_result_inner_failure {
    constexpr auto parsed = phrase(failure_parser_with_movableonly_result())(Input{"input"});
    static_assert(parsed.is_failure());
}

namespace test_phrase_works_with_movable_only_result_phrase_failure {
    constexpr auto parsed = phrase(success_parser_with_movableonly_result())(Input{"input"});
    static_assert(parsed.is_failure());
}

namespace test_phrase_works_with_movable_only_result_phrase_success {
    constexpr auto parsed = phrase(success_parser_with_movableonly_result())(Input{""});
    static_assert(parsed.is_success());
}

TEST(PhraseParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_failure) {
    size_t copy_count = 0, move_count = 0;
    auto parsed = phrase(success_parser_with_copycounting_result(&copy_count, &move_count))(Input{"input"});
    EXPECT_TRUE(parsed.is_failure());
    EXPECT_EQ(0, copy_count);
    EXPECT_EQ(0, move_count);
}

TEST(PhraseParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_success) {
    size_t copy_count = 0, move_count = 0;
    auto parsed = phrase(success_parser_with_copycounting_result(&copy_count, &move_count))(Input{""});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count);
    EXPECT_EQ(1, move_count); // TODO Why isn't this RVO'd?
}

}
