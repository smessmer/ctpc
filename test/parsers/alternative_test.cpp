#include "parsers/alternative.h"
#include "parsers/string.h"

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

}
