#include "parsers/seq.h"
#include "parsers/string.h"
#include "parsers/integer.h"

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
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "CDE");
}
namespace test_seq_two_success {
    constexpr auto parsed = seq(string("ABC"), string("DE"))(Input{"ABCDEFGH"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == std::tuple<std::string_view, std::string_view>{"ABC", "DE"});
    static_assert(parsed.next().input == "FGH");
}
namespace test_seq_two_fail_first {
    constexpr auto parsed = seq(string("ABC"), string("DE"))(Input{"ACDEFGH"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "CDEFGH");
}
namespace test_seq_two_fail_second {
    constexpr auto parsed = seq(string("ABC"), string("DE"))(Input{"ABCDAFGH"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "AFGH");
}
namespace test_seq_three_success {
    constexpr auto parsed = seq(string("ABC"), string("DE"), integer())(Input{"ABCDE23FGH"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == std::tuple<std::string_view, std::string_view, int>{"ABC", "DE", 23});
    static_assert(parsed.next().input == "FGH");
}
namespace test_seq_three_fail_first {
    constexpr auto parsed = seq(string("ABC"), string("DE"), integer())(Input{"ACDE23FGH"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "CDE23FGH");
}
namespace test_seq_three_fail_second {
    constexpr auto parsed = seq(string("ABC"), string("DE"), integer())(Input{"ABCDA23FGH"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "A23FGH");
}
namespace test_seq_three_fail_third {
    constexpr auto parsed = seq(string("ABC"), string("DE"), integer())(Input{"ABCDEFGH"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "FGH");
}

}
