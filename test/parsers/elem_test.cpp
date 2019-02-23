#include "parsers/elem.h"
#include "testutils/move_helpers.h"
#include <gtest/gtest.h>

using namespace ctpc;

namespace {

namespace test_elem_fn_success {
    constexpr auto parsed = elem([] (char) {return true;})(Input{"dandsometext"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'd');
    static_assert(parsed.next().input == "andsometext");
}
namespace test_elem_fn_failure {
    constexpr auto parsed = elem([] (char) {return false;})(Input{"bandsometext"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "bandsometext");
}
namespace test_elem_fn_fatal {
    constexpr auto parsed = elem([] (char) {return true;})(Input{""});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "");
}
namespace test_elem_success {
    constexpr auto parsed = elem('d')(Input{"dandsometext"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'd');
    static_assert(parsed.next().input == "andsometext");
}
namespace test_elem_failure {
    constexpr auto parsed = elem('d')(Input{"bandsometext"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "bandsometext");
}
namespace test_elem_fatal {
    constexpr auto parsed = elem('d')(Input{""});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "");
}

namespace test_elem_fn_movableonly_success {
    constexpr auto parsed = elem(movable_only([] (char) {return true;}))(Input{"dandsometext"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 'd');
    static_assert(parsed.next().input == "andsometext");
}

namespace test_elem_fn_movableonly_failure {
    constexpr auto parsed = elem(movable_only([] (char) {return false;}))(Input{"bandsometext"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "bandsometext");
}

TEST(ElemFnParserTest, doesntCopyOrMoveFunctionMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&& arg) {return elem(std::forward<decltype(arg)>(arg)); },
            {ctpc::Input{""}, ctpc::Input{"abc"}},
            [] () {return [] (char) {return true;}; }
    );
}

}
