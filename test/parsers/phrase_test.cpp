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

TEST(PhraseParserTest, doesntCopyOrMoveMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveMoreThanAbsolutelyNecessary(
            [] (auto&& arg) {return phrase(std::forward<decltype(arg)>(arg)); },
            {ctpc::Input{""}, ctpc::Input{"Found"}, ctpc::Input{"SomethingElse"}, ctpc::Input{"FoundAndSomeMore"}},
            [] () {return string("Found");}
    );
}

}
