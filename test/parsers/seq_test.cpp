#include "parsers/seq.h"
#include "parsers/string.h"
#include "parsers/integer.h"
#include "testutils/move_helpers.h"
#include "testutils/error_parser.h"

using namespace ctpc;

namespace {

namespace test_seq_empty {
    constexpr auto parsed = seq()(Input{"text"});
    static_assert(parsed.is_success());
    static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<decltype(parsed.result())>>, std::tuple<>>);
    static_assert(parsed.result() == std::tuple<>());
    static_assert(parsed.next().input == "text");
}
namespace test_seq_one_success {
    constexpr auto parsed = seq(string("ABC"))(Input{"ABCDE"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == std::tuple<std::string_view>{"ABC"});
    static_assert(parsed.next().input == "DE");
}
namespace test_seq_one_failure {
    constexpr auto parsed = seq(string("ABC"))(Input{"ACDE"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "CDE");
}
namespace test_seq_one_error {
    constexpr auto parsed = seq(error_parser(string("AC"), string("AB")))(Input{"ACDE"});
    static_assert(parsed.is_error());
    static_assert(parsed.next().input == "DE");
}
namespace test_seq_two_success {
    constexpr auto parsed = seq(string("ABC"), string("DE"))(Input{"ABCDEFGH"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == std::tuple<std::string_view, std::string_view>{"ABC", "DE"});
    static_assert(parsed.next().input == "FGH");
}
namespace test_seq_two_failure_first {
    constexpr auto parsed = seq(string("ABC"), string("DE"))(Input{"ACDEFGH"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "CDEFGH");
}
namespace test_seq_two_failure_second {
    constexpr auto parsed = seq(string("ABC"), string("DE"))(Input{"ABCDAFGH"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "AFGH");
}
namespace test_seq_two_error_first {
    constexpr auto parsed = seq(error_parser(string("AC"), string("AB")), string("DE"))(Input{"ACDEFGH"});
    static_assert(parsed.is_error());
    static_assert(parsed.next().input == "DEFGH");
}
namespace test_seq_two_error_second {
    constexpr auto parsed = seq(string("ABC"), error_parser(string("DA"), string("DE")))(Input{"ABCDAFGH"});
    static_assert(parsed.is_error());
    static_assert(parsed.next().input == "FGH");
}
namespace test_seq_three_success {
    constexpr auto parsed = seq(string("ABC"), string("DE"), integer())(Input{"ABCDE23FGH"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == std::tuple<std::string_view, std::string_view, int>{"ABC", "DE", 23});
    static_assert(parsed.next().input == "FGH");
}
namespace test_seq_three_failure_first {
    constexpr auto parsed = seq(string("ABC"), string("DE"), integer())(Input{"ACDE23FGH"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "CDE23FGH");
}
namespace test_seq_three_failure_second {
    constexpr auto parsed = seq(string("ABC"), string("DE"), integer())(Input{"ABCDA23FGH"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "A23FGH");
}
namespace test_seq_three_failure_third {
    constexpr auto parsed = seq(string("ABC"), string("DE"), integer())(Input{"ABCDEFGH"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "FGH");
}
namespace test_seq_three_error_first {
    constexpr auto parsed = seq(error_parser(string("AC"), string("AB")), string("DE"), integer())(Input{"ACDE23FGH"});
    static_assert(parsed.is_error());
    static_assert(parsed.next().input == "DE23FGH");
}
namespace test_seq_three_error_second {
    constexpr auto parsed = seq(string("ABC"), error_parser(string("DA"), string("DE")), integer())(Input{"ABCDA23FGH"});
    static_assert(parsed.is_error());
    static_assert(parsed.next().input == "23FGH");
}
namespace test_seq_three_error_third {
    constexpr auto parsed = seq(string("ABC"), integer(), error_parser(string("FGH"), string("O")))(Input{"ABC23FGHIJK"});
    static_assert(parsed.is_error());
    static_assert(parsed.next().input == "IJK");
}

namespace test_seq_movableonly_success {
    constexpr auto parsed = seq(movable_only(string("ABC")), movable_only(string("DE")), movable_only(integer()))(Input{"ABCDE23FGH"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == std::tuple<std::string_view, std::string_view, int>{"ABC", "DE", 23});
    static_assert(parsed.next().input == "FGH");
}
namespace test_seq_movableonly_failure {
    constexpr auto parsed = seq(movable_only(string("ABC")), movable_only(string("DE")), movable_only(integer()))(Input{"ABCDA23FGH"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "A23FGH");
}
namespace test_seq_movableonly_error {
    constexpr auto parsed = seq(movable_only(error()), movable_only(error()), movable_only(integer()))(Input{"ABCDA23FGH"});
    static_assert(parsed.is_error());
    static_assert(parsed.next().input == "ABCDA23FGH");
}

TEST(SeqParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
        [] (auto&&... args) {return seq(std::forward<decltype(args)>(args)...); },
        {ctpc::Input{""}, ctpc::Input{"ABCDE"}, ctpc::Input{"ABC_"}, ctpc::Input{"ABCDEF"}},
        [] () {return string("AB");},
        [] () {return string("CD");},
        [] () {return string("EF");}
    );
}

namespace test_seq_works_with_movable_only_result {
    constexpr auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    constexpr auto parsed = seq(parser_with_movable_only_result)(Input{"aa"});
    static_assert(parsed.is_success());
}

TEST(SeqParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_success) {
    constexpr auto parser_with_copycounting_result = [](size_t *copy_count, size_t *move_count) {
        return [=](Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_1 = 0, move_count_1 = 0, copy_count_2 = 0, move_count_2 = 0, copy_count_3 = 0, move_count_3 = 0;

    auto parsed = seq(parser_with_copycounting_result(&copy_count_1, &move_count_1), parser_with_copycounting_result(&copy_count_2, &move_count_2), parser_with_copycounting_result(&copy_count_3, &move_count_3))(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_1);
    EXPECT_EQ(2, move_count_1); // Move once when parsed and once when returning the tuple
    EXPECT_EQ(0, copy_count_2);
    EXPECT_EQ(2, move_count_2); // Move once when parsed and once when returning the tuple
    EXPECT_EQ(0, copy_count_3);
    EXPECT_EQ(2, move_count_3); // Move once when parsed and once when returning the tuple
}

TEST(SeqParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_failure) {
    constexpr auto parser_with_copycounting_result = [](size_t *copy_count, size_t *move_count) {
        return [=](Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_1 = 0, move_count_1 = 0, copy_count_2 = 0, move_count_2 = 0, copy_count_3 = 0, move_count_3 = 0;

    auto parsed = seq(parser_with_copycounting_result(&copy_count_1, &move_count_1), parser_with_copycounting_result(&copy_count_2, &move_count_2), parser_with_copycounting_result(&copy_count_3, &move_count_3))(Input{"a"});
    EXPECT_TRUE(parsed.is_failure());
    EXPECT_EQ(0, copy_count_1);
    EXPECT_EQ(1, move_count_1); // Move once when parsed
    EXPECT_EQ(0, copy_count_2);
    EXPECT_EQ(0, move_count_2);
    EXPECT_EQ(0, copy_count_3);
    EXPECT_EQ(0, move_count_3);
}

}
