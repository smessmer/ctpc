#include "parsers/alternative.h"
#include "parsers/string.h"
#include "parsers/elem.h"
#include "parsers/map.h"
#include "testutils/move_helpers.h"

#include <gtest/gtest.h>

using namespace ctpc;

namespace {

namespace test_alternative_empty {
    constexpr auto parsed = alternative()(Input{"sometext"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "sometext");
}
namespace test_alternative_one_success {
    constexpr auto parsed = alternative(string("One"))(Input{"OneAndSomeText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "One");
    static_assert(parsed.next().input == "AndSomeText");
}
namespace test_alternative_one_failure {
    constexpr auto parsed = alternative(string("One"))(Input{"OnAndSomeText"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "AndSomeText");
}
namespace test_alternative_two_success_first {
    constexpr auto parsed = alternative(string("One"), string("OnTwo"))(Input{"OneAndSomeText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "One");
    static_assert(parsed.next().input == "AndSomeText");
}
namespace test_alternative_two_success_second {
    constexpr auto parsed = alternative(string("One"), string("OnTwo"))(Input{"OnTwoAndSomeText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "OnTwo");
    static_assert(parsed.next().input == "AndSomeText");
}
namespace test_alternative_two_failure_first_is_longest_match {
    constexpr auto parsed = alternative(string("Ones"), string("OnTwo"))(Input{"OneThreeAndSomeText"});
    static_assert(!parsed.is_success());
    // return the position from the parser that had the longest match before failing (i.e. "One")
    static_assert(parsed.next().input == "ThreeAndSomeText");
}
namespace test_alternative_two_failure_second_is_longest_match {
    constexpr auto parsed = alternative(string("Ones"), string("OnTwo"))(Input{"OnTwThreeAndSomeText"});
    static_assert(!parsed.is_success());
    // return the position from the parser that had the longest match before failing (i.e. "OnTw")
    static_assert(parsed.next().input == "ThreeAndSomeText");
}
namespace test_alternative_two_failure_all_are_longest_match {
    constexpr auto parsed = alternative(string("Ones"), string("OnTwo"))(Input{"OnSomethingElse"});
    static_assert(!parsed.is_success());
    // return the position from the parser that had the longest match before failing (i.e. "On")
    static_assert(parsed.next().input == "SomethingElse");
}
namespace test_alternative_two_failure_none_is_longest_match {
    constexpr auto parsed = alternative(string("Ones"), string("OnTwo"))(Input{"SomethingElse"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "SomethingElse");
}
namespace test_alternative_three_success_first {
    constexpr auto parsed = alternative(string("One"), string("OnTwo"), string("OnThree"))(Input{"OnesAndSomeText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "One");
    static_assert(parsed.next().input == "sAndSomeText");
}
namespace test_alternative_three_success_second {
    constexpr auto parsed = alternative(string("One"), string("OnTwo"), string("OnThree"))(Input{"OnTwoText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "OnTwo");
    static_assert(parsed.next().input == "Text");
}
namespace test_alternative_three_success_third {
    constexpr auto parsed = alternative(string("One"), string("OnTwo"), string("OnThree"))(Input{"OnThreeText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "OnThree");
    static_assert(parsed.next().input == "Text");
}
namespace test_alternative_three_failure_first_is_longest_match {
    constexpr auto parsed = alternative(string("Ones"), string("OnTwo"), string("OnThree"))(Input{"OneText"});
    static_assert(!parsed.is_success());
    // return the position from the parser that had the longest match before failing (i.e. "One")
    static_assert(parsed.next().input == "Text");
}
namespace test_alternative_three_failure_second_is_longest_match {
    constexpr auto parsed = alternative(string("Ones"), string("OnTwo"), string("OnThree"))(Input{"OnTwThree"});
    static_assert(!parsed.is_success());
    // return the position from the parser that had the longest match before failing (i.e. "OneTw")
    static_assert(parsed.next().input == "Three");
}
namespace test_alternative_three_failure_third_is_longest_match {
    constexpr auto parsed = alternative(string("Ones"), string("OnTwo"), string("OnThree"))(Input{"OnThreText"});
    static_assert(!parsed.is_success());
    // return the position from the parser that had the longest match before failing (i.e. "OnThre")
    static_assert(parsed.next().input == "Text");
}
namespace test_alternative_three_failure_all_are_longest_match {
    constexpr auto parsed = alternative(string("Ones"), string("OnTwo"), string("OnThree"))(Input{"OnSomethingElse"});
    static_assert(!parsed.is_success());
    // return the position from the parser that had the longest match before failing (i.e. "On")
    static_assert(parsed.next().input == "SomethingElse");
}
namespace test_alternative_three_failure_none_is_longest_match {
    constexpr auto parsed = alternative(string("Ones"), string("OnTwo"), string("OnThree"))(Input{"SomethingElse"});
    static_assert(!parsed.is_success());
    // return the position from the parser that had the longest match before failing (i.e. "On")
    static_assert(parsed.next().input == "SomethingElse");
}

namespace test_alternative_works_with_movable_only_parsers_success {
    constexpr auto parsed = alternative(movable_only(string("One")), movable_only(string("OnTwo")))(Input{"OnTwoAndSomeText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == "OnTwo");
    static_assert(parsed.next().input == "AndSomeText");
}

namespace test_alternative_works_with_movable_only_parsers_failure {
    constexpr auto parsed = alternative(movable_only(string("Ones")), movable_only(string("OnTwo")), movable_only(string("OnThree")))(Input{"OnTwThree"});
    static_assert(!parsed.is_success());
    // return the position from the parser that had the longest match before failing (i.e. "OneTw")
    static_assert(parsed.next().input == "Three");
}

TEST(AlternativeParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return alternative(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"OnTwoSomeText"}, ctpc::Input{"SomethingElse"}},
            [] () {return string("One");},
            [] () {return string("OnTwo");},
            [] () {return string("OnThree");}
    );
}

namespace test_alternative_works_with_movable_only_result_success {
    constexpr auto parsed = alternative(failure_parser_with_movableonly_result(), success_parser_with_movableonly_result(), success_parser_with_movableonly_result())(Input{"input"});
    static_assert(parsed.is_success());
}

namespace test_alternative_works_with_movable_only_result_failure {
    constexpr auto parsed = alternative(failure_parser_with_movableonly_result(), failure_parser_with_movableonly_result(), failure_parser_with_movableonly_result())(Input{"input"});
    static_assert(!parsed.is_success());
}

TEST(AlternativeParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary) {
    size_t copy_count = 0, move_count = 0;
    auto parsed = alternative(failure_parser_with_copycounting_result(), success_parser_with_copycounting_result(&copy_count, &move_count), success_parser_with_copycounting_result(&copy_count, &move_count))(Input{"input"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count);
    EXPECT_EQ(1, move_count); // TODO Can this be further optimized?
}

namespace test_alternative_works_with_convertible_types_intdouble {
    constexpr auto int_parser = mapValue<int>(elem('a'), int(1));
    constexpr auto double_parser = mapValue<double>(elem('b'), double(2));

    constexpr auto parser1 = alternative(int_parser, double_parser);
    static_assert(std::is_same_v<double, parser_result_t<decltype(parser1)>>);
    constexpr auto parsed1_1 = parser1(Input{"a"}).result();
    static_assert(std::is_same_v<const double, decltype(parsed1_1)>);
    static_assert(1 == parsed1_1);
    constexpr auto parsed1_2 = parser1(Input{"b"}).result();
    static_assert(std::is_same_v<const double, decltype(parsed1_2)>);
    static_assert(2 == parsed1_2);
}

namespace test_alternative_works_with_convertible_types_doubleintfloat {
    constexpr auto int_parser = mapValue<int>(elem('a'), int(1));
    constexpr auto double_parser = mapValue<double>(elem('b'), double(2));
    constexpr auto float_parser = mapValue<float>(elem('c'), float(3));

    constexpr auto parser2 = alternative(double_parser, int_parser, float_parser);
    static_assert(std::is_same_v<double, parser_result_t<decltype(parser2)>>);
    constexpr auto parsed2_1 = parser2(Input{"a"}).result();
    static_assert(std::is_same_v<const double, decltype(parsed2_1)>);
    static_assert(1 == parsed2_1);
    constexpr auto parsed2_2 = parser2(Input{"b"}).result();
    static_assert(std::is_same_v<const double, decltype(parsed2_2)>);
    static_assert(2 == parsed2_2);
    constexpr auto parsed2_3 = parser2(Input{"c"}).result();
    static_assert(std::is_same_v<const double, decltype(parsed2_3)>);
    static_assert(3 == parsed2_3);
}

}
