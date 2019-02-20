#include "parsers/rep.h"
#include "parsers/string.h"
#include "parsers/integer.h"
#include "testutils/move_helpers.h"
#include <gtest/gtest.h>

using namespace ctpc;

namespace {

namespace test_repsep_type {
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep(string("Elem"), string("sep"))(std::declval<Input>()).result())>>, cvector<std::string_view, 1024>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep(integer(), string("sep"))(std::declval<Input>()).result())>>, cvector<long long, 1024>>::value);
}
namespace test_repsep_empty {
    constexpr auto parsed = repsep(string("Elem"), string("Sep"))(Input{""});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 0);
    static_assert(parsed.next().input == "");
}
namespace test_repsep_none {
    constexpr auto parsed = repsep(string("Elem"), string("Sep"))(Input{"El"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 0);
    static_assert(parsed.next().input == "El");
}
namespace test_repsep_one {
    constexpr auto parsed = repsep(string("Elem"), string("Sep"))(Input{"ElemSa"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 1);
    static_assert(parsed.result()[0] == "Elem");
    static_assert(parsed.next().input == "Sa");
}
namespace test_repsep_one_following_sep {
    constexpr auto parsed = repsep(string("Elem"), string("Sep"))(Input{"ElemSep"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 1);
    static_assert(parsed.result()[0] == "Elem");
    static_assert(parsed.next().input == "Sep"); // last separator doesn't count, stays in input.
}
namespace test_repsep_two {
    constexpr auto parsed = repsep(integer(), string("Sep"))(Input{"23Sep39Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 2);
    static_assert(parsed.result()[0] == 23);
    static_assert(parsed.result()[1] == 39);
    static_assert(parsed.next().input == "Text");
}
namespace test_repsep_two_following_sep {
    constexpr auto parsed = repsep(integer(), string("Sep"))(Input{"23Sep39Sep"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 2);
    static_assert(parsed.result()[0] == 23);
    static_assert(parsed.result()[1] == 39);
    static_assert(parsed.next().input == "Sep"); // last separator doesn't count, stays in input
}
namespace test_repsep_three {
    constexpr auto parsed = repsep(integer(), string("Sep"))(Input{"23Sep39Sep358Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 3);
    static_assert(parsed.result()[0] == 23);
    static_assert(parsed.result()[1] == 39);
    static_assert(parsed.result()[2] == 358);
    static_assert(parsed.next().input == "Text");
}
namespace test_repsep_three_following_sep {
    constexpr auto parsed = repsep(integer(), string("Sep"))(Input{"23Sep39Sep358SepSomething"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 3);
    static_assert(parsed.result()[0] == 23);
    static_assert(parsed.result()[1] == 39);
    static_assert(parsed.result()[2] == 358);
    static_assert(parsed.next().input == "SepSomething"); // last separator doesn't count, stays in input
}
namespace test_rep_type {
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep(string("Elem"))(std::declval<Input>()).result())>>, cvector<std::string_view, 1024>>::value);
}
namespace test_rep_empty {
    constexpr auto parsed = rep(string("Elem"))(Input{""});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 0);
    static_assert(parsed.next().input == "");
}
namespace test_rep_none {
    constexpr auto parsed = rep(string("Elem"))(Input{"El"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 0);
    static_assert(parsed.next().input == "El");
}
namespace test_rep_one {
    constexpr auto parsed = rep(string("Elem"))(Input{"ElemSa"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 1);
    static_assert(parsed.result()[0] == "Elem");
    static_assert(parsed.next().input == "Sa");
}
namespace test_rep_two {
    constexpr auto parsed = rep(string("Elem"))(Input{"ElemElemElText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 2);
    static_assert(parsed.result()[0] == "Elem");
    static_assert(parsed.result()[1] == "Elem");
    static_assert(parsed.next().input == "ElText"); // doesn't parse last 'El' even though that partly matches, because it's not a failure result.
}
namespace test_rep_three {
    constexpr auto parsed = rep(string("Elem"))(Input{"ElemElemElemElText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 3);
    static_assert(parsed.result()[0] == "Elem");
    static_assert(parsed.result()[1] == "Elem");
    static_assert(parsed.result()[2] == "Elem");
    static_assert(parsed.next().input == "ElText"); // doesn't parse last 'El' even though that partly matches, because it's not a failure result.
}
namespace test_rep1_type {
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep1(string("Elem"))(std::declval<Input>()).result())>>, cvector<std::string_view, 1024>>::value);
}
namespace test_rep1_empty {
    constexpr auto parsed = rep1(string("Elem"))(Input{""});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "");
}
namespace test_rep1_none {
    constexpr auto parsed = rep1(string("Elem"))(Input{"ElAb"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "ElAb"); // TODO Should this be "Ab" instead? Check Scala parser combinators.
}
namespace test_rep1_one {
    constexpr auto parsed = rep1(string("Elem"))(Input{"ElemSa"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 1);
    static_assert(parsed.result()[0] == "Elem");
    static_assert(parsed.next().input == "Sa");
}
namespace test_rep1_two {
    constexpr auto parsed = rep1(string("Elem"))(Input{"ElemElemElText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 2);
    static_assert(parsed.result()[0] == "Elem");
    static_assert(parsed.result()[1] == "Elem");
    static_assert(parsed.next().input == "ElText"); // doesn't parse last 'El' even though that partly matches, because it's not a failure result.
}
namespace test_rep1_three {
    constexpr auto parsed = rep1(string("Elem"))(Input{"ElemElemElemElText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 3);
    static_assert(parsed.result()[0] == "Elem");
    static_assert(parsed.result()[1] == "Elem");
    static_assert(parsed.result()[2] == "Elem");
    static_assert(parsed.next().input == "ElText"); // doesn't parse last 'El' even though that partly matches, because it's not a failure result.
}

namespace test_repsep_movableonly_none {
    constexpr auto parsed = repsep(movable_only(string("Elem")), movable_only(string("Sep")))(Input{"El"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 0);
    static_assert(parsed.next().input == "El");
}
namespace test_repsep_movableonly_two {
    constexpr auto parsed = repsep(movable_only(integer()), movable_only(string("Sep")))(Input{"23Sep39Text"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 2);
    static_assert(parsed.result()[0] == 23);
    static_assert(parsed.result()[1] == 39);
    static_assert(parsed.next().input == "Text");
}
namespace test_rep_movableonly_none {
    constexpr auto parsed = rep(movable_only(string("Elem")))(Input{"El"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 0);
    static_assert(parsed.next().input == "El");
}
namespace test_rep_movableonly_two {
    constexpr auto parsed = rep(movable_only(string("Elem")))(Input{"ElemElemElText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 2);
    static_assert(parsed.result()[0] == "Elem");
    static_assert(parsed.result()[1] == "Elem");
    static_assert(parsed.next().input == "ElText"); // doesn't parse last 'El' even though that partly matches, because it's not a failure result.
}
namespace test_rep1_movableonly_none {
    constexpr auto parsed = rep1(movable_only(string("Elem")))(Input{"ElAb"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "ElAb"); // TODO Should this be "Ab" instead? Check Scala parser combinators.
}
namespace test_rep1_movableonly_two {
    constexpr auto parsed = rep1(movable_only(string("Elem")))(Input{"ElemElemElText"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().size() == 2);
    static_assert(parsed.result()[0] == "Elem");
    static_assert(parsed.result()[1] == "Elem");
    static_assert(parsed.next().input == "ElText"); // doesn't parse last 'El' even though that partly matches, because it's not a failure result.
}

TEST(RepSepParserTest, doesntCopyOrMoveMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return repsep(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"Found,Found"}, ctpc::Input{"Found,FoundBla"}, ctpc::Input{"Found,Found,"}, ctpc::Input{"Found,Found,Bla"}},
            [] () {return string("Found");},
            [] () {return string(",");}
    );
}

}
