#include "parsers/map.h"
#include "parsers/string.h"
#include "parsers/basic_parsers.h"
#include "parsers/elem.h"
#include "testutils/move_helpers.h"
#include <gtest/gtest.h>

using namespace ctpc;

namespace {

namespace test_flatmap_success_to_success {
    constexpr auto parser = flatMap(string("123"), [] (auto parsed) {
        if (parsed.is_success() && parsed.result() == "123") {
            return ParseResult<long long>::success(parsed.next(), 123);
        } else {
            return ParseResult<long long>::failure(parsed.next());
        }
    });
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12345"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "45");
}
namespace test_flatmap_success_to_failure {
    constexpr auto parser = flatMap(string("123"), [] (auto parsed) {
        if (parsed.is_success() && parsed.result() == "123") {
            return ParseResult<long long>::failure(Input{"next"});
        } else {
            return ParseResult<long long>::success(parsed.next(), 0);
        }
    });
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12345"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "next");
}
namespace test_flatmap_failure_to_success {
    constexpr auto parser = flatMap(string("123"), [] (auto parsed) {
        if (parsed.is_success()) {
            return ParseResult<long long>::failure(parsed.next());
        } else {
            return ParseResult<long long>::success(Input{"next"}, 123);
        }
    });
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"unparseable"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "next");
}
namespace test_flatmap_failure_to_failure {
    constexpr auto parser = flatMap(string("123"), [] (auto parsed) {
        if (parsed.is_success()) {
            return ParseResult<long long>::success(parsed.next(), 0);
        } else {
            return ParseResult<long long>::failure(Input{"next"});
        }
    });
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"unparseable"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "next");
}
namespace test_map_success {
    constexpr auto parser = map(string("123"), [] (auto parsed) -> long long {
        if (parsed == "123") {
            return 123;
        } else {
            return 0;
        }
    });
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12345"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "45");
}
namespace test_map_failure {
    constexpr auto parser = map(string("123"), [] (auto /*parsed*/) -> long long {
        throw std::logic_error("This shouldn't be called");
    });
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12unparseable"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "unparseable");
}
namespace test_mapvalue_success {
    constexpr auto parser = mapValue(string("123"), static_cast<long long>(123));
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12345"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "45");
}
namespace test_mapvalue_failure {
    constexpr auto parser = mapValue(string("123"), static_cast<long long>(123));
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12unparseable"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "unparseable");
}


namespace test_flatmap_movableonly_success_to_success {
    constexpr auto parser = flatMap(movable_only(string("123")), movable_only([] (auto parsed) {
        if (parsed.is_success() && parsed.result() == "123") {
            return ParseResult<long long>::success(parsed.next(), 123);
        } else {
            return ParseResult<long long>::failure(parsed.next());
        }
    }));
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12345"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "45");
}
namespace test_flatmap_movableonly_success_to_failure {
    constexpr auto parser = flatMap(movable_only(string("123")), movable_only([] (auto parsed) {
        if (parsed.is_success() && parsed.result() == "123") {
            return ParseResult<long long>::failure(Input{"next"});
        } else {
            return ParseResult<long long>::success(parsed.next(), 0);
        }
    }));
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12345"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "next");
}
namespace test_flatmap_movableonly_failure_to_success {
    constexpr auto parser = flatMap(movable_only(string("123")), movable_only([] (auto parsed) {
        if (parsed.is_success()) {
            return ParseResult<long long>::failure(parsed.next());
        } else {
            return ParseResult<long long>::success(Input{"next"}, 123);
        }
    }));
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"unparseable"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "next");
}
namespace test_flatmap_movableonly_failure_to_failure {
    constexpr auto parser = flatMap(movable_only(string("123")), movable_only([] (auto parsed) {
        if (parsed.is_success()) {
            return ParseResult<long long>::success(parsed.next(), 0);
        } else {
            return ParseResult<long long>::failure(Input{"next"});
        }
    }));
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"unparseable"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "next");
}
namespace test_map_movableonly_success {
    constexpr auto parser = map(movable_only(string("123")), movable_only([] (auto parsed) -> long long {
        if (parsed == "123") {
            return 123;
        } else {
            return 0;
        }
    }));
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12345"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "45");
}
namespace test_map_movableonly_failure {
    constexpr auto parser = map(movable_only(string("123")), movable_only([] (auto /*parsed*/) -> long long {
        throw std::logic_error("This shouldn't be called");
    }));
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12unparseable"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "unparseable");
}
namespace test_mapvalue_movableonly_success {
    constexpr auto parser = mapValue(movable_only(string("123")), static_cast<long long>(123));
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12345"});
    static_assert(parsed.is_success());
    static_assert(parsed.result() == 123);
    static_assert(parsed.next().input == "45");
}
namespace test_mapvalue_movableonly_failure {
    constexpr auto parser = mapValue(movable_only(string("123")), static_cast<long long>(123));
    static_assert(std::is_same_v<parser_result_t<decltype(parser)>, long long>);
    constexpr auto parsed = parser(Input{"12unparseable"});
    static_assert(!parsed.is_success());
    static_assert(parsed.next().input == "unparseable");
}

TEST(FlatMapParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return flatMap(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return [] (auto a) {return a;}; }
    );

    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return flatMap(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return [] (auto a) {return ParseResult<int>::success(a.next(), 3);}; }
    );

    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return flatMap(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return [] (auto a) {return ParseResult<int>::failure(a.next());}; }
    );

    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return flatMap(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return [] (auto a) {return ParseResult<int>::error(a.next());}; }
    );
}

TEST(MapParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return map(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return [] (auto a) {return a;}; }
    );
}

TEST(MapValueParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&& arg) {return mapValue(std::forward<decltype(arg)>(arg), 5); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");}
    );
}

namespace test_flatmap_works_with_movable_only_result_success {
    constexpr auto parsed = flatMap(success_parser_with_movableonly_result(), [] (auto res) {return res;})(Input{"input"});
    static_assert(parsed.is_success());
}

namespace test_flatmap_works_with_movable_only_result_failure {
    constexpr auto parsed = flatMap(failure_parser_with_movableonly_result(), [] (auto res) {return res;})(Input{"input"});
    static_assert(parsed.is_failure());
}

TEST(FlatMapParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_newobjects) {
    size_t copy_count = 0, move_count = 0;
    auto parsed = flatMap(success_parser_with_copycounting_result(&copy_count, &move_count), [&] (auto res) {
        return ParseResult<copy_counting<double>>::success(res.next(), 4, &copy_count, &move_count);
    })(Input{"input"});

    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count);
    EXPECT_EQ(0, move_count);
}

TEST(FlatMapParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_passthrough) {
    size_t copy_count = 0, move_count = 0;
    auto parsed = flatMap(success_parser_with_copycounting_result(&copy_count, &move_count), [] (auto res) {return res;})(Input{"input"});

    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count);
    EXPECT_EQ(1, move_count);
}

namespace test_map_works_with_movable_only_result_success {
    constexpr auto parsed = map(success_parser_with_movableonly_result(), [] (auto res) {return res;})(Input{"input"});
    static_assert(parsed.is_success());
}

namespace test_map_works_with_movable_only_result_failure {
    constexpr auto parsed = map(failure_parser_with_movableonly_result(), [] (auto res) {return res;})(Input{"input"});
    static_assert(parsed.is_failure());
}

TEST(MapParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_newobjects) {
    size_t copy_count_1 = 0, move_count_1 = 0, copy_count_2 = 0, move_count_2 = 0;
    auto parsed = map(success_parser_with_copycounting_result(&copy_count_1, &move_count_1), [&] (auto&&) {
        return copy_counting<double>(4, &copy_count_2, &move_count_2);
    })(Input{"input"});

    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_1);
    EXPECT_EQ(0, move_count_1);
    EXPECT_EQ(0, copy_count_2);
    EXPECT_EQ(1, move_count_2); // TODO Why does this need to be moved?
}

TEST(MapParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_passthrough) {
    size_t copy_count = 0, move_count = 0;
    auto parsed = map(success_parser_with_copycounting_result(&copy_count, &move_count), [] (auto&& res) {return std::move(res);})(Input{"input"});

    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count);
    EXPECT_EQ(2, move_count);
}

TEST(MapValueParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary) {
    size_t copy_count_1 = 0, move_count_1 = 0, copy_count_2 = 0, move_count_2 = 0;
    auto parser = mapValue(success_parser_with_copycounting_result(&copy_count_1, &move_count_1), copy_counting<double>(4, &copy_count_2, &move_count_2));
    EXPECT_EQ(0, copy_count_1);
    EXPECT_EQ(0, move_count_1);
    EXPECT_EQ(0, copy_count_2);
    EXPECT_EQ(1, move_count_2);

    auto parsed = parser(Input{"input"});

    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_1);
    EXPECT_EQ(0, move_count_1);
    EXPECT_EQ(1, copy_count_2);
    EXPECT_EQ(1, move_count_2);
}

}
