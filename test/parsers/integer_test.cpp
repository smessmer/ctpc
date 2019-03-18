#include "parsers/integer.h"
#include <gtest/gtest.h>

using namespace ctpc;

namespace {

namespace test_numeric_type {
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(numeric()(std::declval<Input>()).result())>>, char>::value);
}
namespace test_numeric_failure {
    constexpr auto parsed = numeric()(Input{"a123"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "a123");
}
namespace test_numeric_success_0 {
    constexpr auto parsed = numeric()(Input{"0123456789"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == '0');
    static_assert(parsed.next().input == "123456789");
}
namespace test_numeric_success_5 {
    constexpr auto parsed = numeric()(Input{"56789"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == '5');
    static_assert(parsed.next().input == "6789");
}
namespace test_numeric_success_9 {
    constexpr auto parsed = numeric()(Input{"9123"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == '9');
    static_assert(parsed.next().input == "123");
}
namespace test_digit_type {
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(digit()(std::declval<Input>()).result())>>, uint8_t>::value);
}
namespace test_digit_failure {
    constexpr auto parsed = digit()(Input{"a123"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "a123");
}
namespace test_digit_success_0 {
    constexpr auto parsed = digit()(Input{"0123456789"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 0);
    static_assert(parsed.next().input == "123456789");
}
namespace test_digit_success_5 {
    constexpr auto parsed = digit()(Input{"56789"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 5);
    static_assert(parsed.next().input == "6789");
}
namespace test_digit_success_9 {
    constexpr auto parsed = digit()(Input{"9123"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 9);
    static_assert(parsed.next().input == "123");
}
namespace test_integer_failure {
    constexpr auto parsed = integer()(Input{"dsometext"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "dsometext");
}
TEST(IntegerTest, failure) {
    auto parsed = integer()(Input{"dsometext"});
    EXPECT_FALSE(parsed.is_success());
    EXPECT_EQ("dsometext", parsed.next().input);
}
namespace test_integer_success {
    constexpr auto parsed = integer()(Input{"343894andsometext"});
    static_assert(parsed.is_success());
    static_assert(343894 == parsed.result());
    static_assert(parsed.next().input == "andsometext");
}
TEST(IntegerTest, success) {
    auto parsed = integer()(Input{"343894andsometext"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(343894, parsed.result());
    EXPECT_EQ("andsometext", parsed.next().input);
}

}
