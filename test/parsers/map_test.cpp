#include "parsers/map.h"
#include "parsers/string.h"

using namespace ctpc;

namespace {

namespace test_flatmap_success_to_success {
    constexpr auto parser() {
        return flatMap(string("123"), [] (auto parsed) {
            if (parsed.is_success() && parsed.result() == "123") {
                return ParseResult<long long>::success(123, parsed.next());
            } else {
                return ParseResult<long long>::failure(parsed.next());
            }
        });
    }
    static_assert(std::is_same<parser_result_t<decltype(parser())>, long long>::value);
    constexpr auto parsed = parser()(Input{"12345"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "45");
}
namespace test_flatmap_success_to_failure {
    constexpr auto parser() {
        return flatMap(string("123"), [] (auto parsed) {
            if (parsed.is_success() && parsed.result() == "123") {
                return ParseResult<long long>::failure(Input{"next"});
            } else {
                return ParseResult<long long>::success(0, parsed.next());
            }
        });
    }
    static_assert(std::is_same<parser_result_t<decltype(parser())>, long long>::value);
    constexpr auto parsed = parser()(Input{"12345"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "next");
}
namespace test_flatmap_failure_to_success {
    constexpr auto parser() {
        return flatMap(string("123"), [] (auto parsed) {
            if (parsed.is_success()) {
                return ParseResult<long long>::failure(parsed.next());
            } else {
                return ParseResult<long long>::success(123, Input{"next"});
            }
        });
    }
    static_assert(std::is_same<parser_result_t<decltype(parser())>, long long>::value);
    constexpr auto parsed = parser()(Input{"unparseable"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "next");
}
namespace test_flatmap_failure_to_failure {
    constexpr auto parser() {
        return flatMap(string("123"), [] (auto parsed) {
            if (parsed.is_success()) {
                return ParseResult<long long>::success(0, parsed.next());
            } else {
                return ParseResult<long long>::failure(Input{"next"});
            }
        });
    }
    static_assert(std::is_same<parser_result_t<decltype(parser())>, long long>::value);
    constexpr auto parsed = parser()(Input{"unparseable"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "next");
}
namespace test_map_success {
    constexpr auto parser() {
        return map(string("123"), [] (auto parsed) -> long long {
            if (parsed == "123") {
                return 123;
            } else {
                return 0;
            }
        });
    }
    static_assert(std::is_same<parser_result_t<decltype(parser())>, long long>::value);
    constexpr auto parsed = parser()(Input{"12345"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "45");
}
namespace test_map_failure {
    constexpr auto parser() {
        return map(string("123"), [] (auto /*parsed*/) -> long long {
            throw std::logic_error("This shouldn't be called");
        });
    }
    static_assert(std::is_same<parser_result_t<decltype(parser())>, long long>::value);
    constexpr auto parsed = parser()(Input{"12unparseable"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "unparseable");
}
namespace test_mapvalue_success {
    constexpr auto parser() {
        return mapValue(string("123"), static_cast<long long>(123));
    }
    static_assert(std::is_same<parser_result_t<decltype(parser())>, long long>::value);
    constexpr auto parsed = parser()(Input{"12345"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "45");
}
namespace test_mapvalue_failure {
    constexpr auto parser() {
        return mapValue(string("123"), static_cast<long long>(123));
    }
    static_assert(std::is_same<parser_result_t<decltype(parser())>, long long>::value);
    constexpr auto parsed = parser()(Input{"12unparseable"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "unparseable");
}

}
