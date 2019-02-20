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
            return ParseResult<long long>::success(123, parsed.next());
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
            return ParseResult<long long>::success(0, parsed.next());
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
            return ParseResult<long long>::success(123, Input{"next"});
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
            return ParseResult<long long>::success(0, parsed.next());
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
            return ParseResult<long long>::success(123, parsed.next());
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
            return ParseResult<long long>::success(0, parsed.next());
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
            return ParseResult<long long>::success(123, Input{"next"});
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
            return ParseResult<long long>::success(0, parsed.next());
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

TEST(FlatMapParserTest, doesntCopyOrMoveMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return flatMap(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return [] (auto a) {return a;}; }
    );

    testDoesntCopyOrMoveMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return flatMap(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return [] (auto a) {return ParseResult<int>::success(3, a.next());}; }
    );

    testDoesntCopyOrMoveMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return flatMap(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return [] (auto a) {return ParseResult<int>::failure(a.next());}; }
    );

    testDoesntCopyOrMoveMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return flatMap(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return [] (auto a) {return ParseResult<int>::error(a.next());}; }
    );
}

TEST(MapParserTest, doesntCopyOrMoveMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return map(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return [] (auto a) {return a;}; }
    );
}

/* TODO Use this instead of the MapValueParser_CountCopyAndMove below once perf is fixed
TEST(MapValueParserTest, doesntCopyOrMoveMoreThanAbsolutelyNecessary) {
    testDoesntCopyOrMoveMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return mapValue(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundBla"}, ctpc::Input{"NotFound"}},
            [] () {return string("Found");},
            [] () {return 5;}
    );
}*/

TEST(MapValueParser_CountCopyAndMove, constructionFromTemporary) {
    size_t parser_copy_count = 0, parser_move_count = 0, result_copy_count = 0, result_move_count = 0;
    auto parser = mapValue(copy_counting(success(), &parser_copy_count, &parser_move_count), copy_counting(5, &result_copy_count, &result_move_count));
    EXPECT_EQ(0, parser_copy_count);
    EXPECT_EQ(1, parser_move_count);
    EXPECT_EQ(0, result_copy_count);
    EXPECT_EQ(1, result_move_count);
    (void)(parser); // fix unused parameter warning
}

TEST(MapValueParser_CountCopyAndMove, constructionFromRvalue) {
    size_t parser_copy_count = 0, parser_move_count = 0, result_copy_count = 0, result_move_count = 0;
    copy_counting value(5, &result_copy_count, &result_move_count);
    copy_counting parser1(success(), &parser_copy_count, &parser_move_count);
    auto parser = mapValue(std::move(parser1), std::move(value));

    EXPECT_EQ(0, parser_copy_count);
    EXPECT_EQ(1, parser_move_count);
    EXPECT_EQ(0, result_copy_count);
    EXPECT_EQ(1, result_move_count);
    (void)(parser); // fix unused parameter warning
}

TEST(MapValueParser_CountCopyAndMove, constructionFromLvalue) {
    size_t parser_copy_count = 0, parser_move_count = 0, result_copy_count = 0, result_move_count = 0;
    copy_counting value(5, &result_copy_count, &result_move_count);
    copy_counting parser1(success(), &parser_copy_count, &parser_move_count);
    auto parser = mapValue(parser1, value);

    EXPECT_EQ(1, parser_copy_count);
    EXPECT_EQ(0, parser_move_count);
    EXPECT_EQ(1, result_copy_count);
    EXPECT_EQ(0, result_move_count);
    (void)(parser); // fix unused parameter warning
}

TEST(MapValueParser_CountCopyAndMove, calling) {
    size_t parser_copy_count = 0, parser_move_count = 0, result_copy_count = 0, result_move_count = 0;
    auto parser = mapValue(copy_counting(elem('a'), &parser_copy_count, &parser_move_count), copy_counting(5, &result_copy_count, &result_move_count));
    parser_copy_count = parser_move_count = result_copy_count = result_move_count = 0;

    // calling success case doesn't copy or move
    auto parsed = parser(Input{"a"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, parser_copy_count);
    EXPECT_EQ(0, parser_move_count);
    EXPECT_EQ(2, result_copy_count); // TODO Why not 1? Optimize!
    EXPECT_EQ(1, result_move_count); // TODO Why not 0? Optimize!

    // calling failure case doesn't copy or move
    parsed = parser(Input{"b"});
    EXPECT_TRUE(parsed.is_failure());
    EXPECT_EQ(0, parser_copy_count);
    EXPECT_EQ(0, parser_move_count);
    EXPECT_EQ(2, result_copy_count); // TODO Why not 0? Optimize!
    EXPECT_EQ(1, result_move_count); // TODO Why not 0? Optimize!
}

TEST(MapValueParser_CountCopyAndMove, copying) {
    size_t parser_copy_count = 0, parser_move_count = 0, result_copy_count = 0, result_move_count = 0;
    auto parser = mapValue(copy_counting(elem('a'), &parser_copy_count, &parser_move_count), copy_counting(5, &result_copy_count, &result_move_count));
    parser_copy_count = parser_move_count = result_copy_count = result_move_count = 0;

    auto copy = parser;
    EXPECT_EQ(1, parser_copy_count);
    EXPECT_EQ(0, parser_move_count);
    EXPECT_EQ(1, result_copy_count);
    EXPECT_EQ(0, result_move_count);
}

TEST(MapValueParser_CountCopyAndMove, moving) {
    size_t parser_copy_count = 0, parser_move_count = 0, result_copy_count = 0, result_move_count = 0;
    auto parser = mapValue(copy_counting(elem('a'), &parser_copy_count, &parser_move_count), copy_counting(5, &result_copy_count, &result_move_count));
    parser_copy_count = parser_move_count = result_copy_count = result_move_count = 0;

    auto copy = std::move(parser);
    EXPECT_EQ(0, parser_copy_count);
    EXPECT_EQ(1, parser_move_count);
    EXPECT_EQ(0, result_copy_count);
    EXPECT_EQ(1, result_move_count);
}

}
