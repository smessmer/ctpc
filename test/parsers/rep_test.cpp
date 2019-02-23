#include "parsers/rep.h"
#include "parsers/string.h"
#include "parsers/integer.h"
#include "testutils/move_helpers.h"
#include <gtest/gtest.h>

using namespace ctpc;

namespace {

namespace test_repsep_type {
    // compiletime / runtime overloads
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep(compiletime_optimization, string("Elem"), string("sep"))(std::declval<Input>()).result())>>, cvector<std::string_view, 1024>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep(compiletime_optimization, integer(), string("sep"))(std::declval<Input>()).result())>>, cvector<long long, 1024>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep(runtime_optimization, string("Elem"), string("sep"))(std::declval<Input>()).result())>>, std::vector<std::string_view>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep(runtime_optimization, integer(), string("sep"))(std::declval<Input>()).result())>>, std::vector<long long>>::value);

    // custom container variant
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep<std::vector<std::string_view>>(string("Elem"), string("sep"))(std::declval<Input>()).result())>>, std::vector<std::string_view>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep<std::vector<long long>>(integer(), string("sep"))(std::declval<Input>()).result())>>, std::vector<long long>>::value);

    // custom container variant with parser and container having non-matching but convertible result types   
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep<std::vector<std::string>>(string("Elem"), string("sep"))(std::declval<Input>()).result())>>, std::vector<std::string>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep<std::vector<uint8_t>>(integer(), string("sep"))(std::declval<Input>()).result())>>, std::vector<uint8_t>>::value);

    // accumulator variant
    constexpr auto initAccumulator = [] () -> int {return 0; };
    constexpr auto handleElement = [] (int* /*accumulator*/, std::string_view /*element*/) {};
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep(string("Elem"), string("sep"), initAccumulator, handleElement)(std::declval<Input>()).result())>>, int>::value);
}

constexpr void expectTrue_(bool value) {
    if (!value) {
        throw std::runtime_error("Test failed");
    }
}

template<class ElementParserCreator, class SeparatorParserCreator, class Expectation>
class RepsepTestCase final {
public:
    using ElementParser = std::decay_t<decltype(std::declval<ElementParserCreator>()())>;
    using SeparatorParser = std::decay_t<decltype(std::declval<SeparatorParserCreator>()())>;

    constexpr RepsepTestCase(ElementParserCreator elementParserCreator, SeparatorParserCreator separatorParserCreator, Input input, Expectation expectation)
    : elementParserCreator_(std::move(elementParserCreator))
    , separatorParserCreator_(std::move(separatorParserCreator))
    , input_(input)
    , expectation_(std::move(expectation))
    {}

    constexpr bool test_all_compiletime() {
        test_compiletime_();
        test_compiletime_customcontainer_();
        test_compiletime_customcontainer_reservecapacity_();
        test_compiletime_accumulator_();

        return true;
    }

    void test_all_runtime() {
        test_runtime_cvector_();
        test_runtime_stdvector_();
        test_runtime_stdvector_reservecapacity_();
        test_runtime_customcontainer_cvector_();
        test_runtime_customcontainer_cvector_reservecapacity_();
        test_runtime_customcontainer_stdvector_();
        test_runtime_customcontainer_stdvector_reservecapacity_();
        test_runtime_accumulator_cvector_();
        test_runtime_accumulator_stdvector_();
    }

private:

    constexpr void test_compiletime_() {
        auto parsed = repsep(compiletime_optimization, elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_cvector_() {
        auto parsed = repsep(compiletime_optimization, elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_stdvector_() {
        auto parsed = repsep(runtime_optimization, elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_stdvector_reservecapacity_() {
        auto parsed = repsep(runtime_optimization, elementParserCreator_(), separatorParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_customcontainer_() {
        auto parsed = repsep<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_customcontainer_reservecapacity_() {
        auto parsed = repsep<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), separatorParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_cvector_() {
        auto parsed = repsep<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_stdvector_() {
        auto parsed = repsep<std::vector<parser_result_t<ElementParser>>>(elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_cvector_reservecapacity_() {
        auto parsed = repsep<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), separatorParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_stdvector_reservecapacity_() {
        auto parsed = repsep<std::vector<parser_result_t<ElementParser>>>(elementParserCreator_(), separatorParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_accumulator_() {
        constexpr auto initAccumulator = [] () {return cvector<parser_result_t<ElementParser>, 1024>(); };
        constexpr auto handleElement = [] (cvector<parser_result_t<ElementParser>, 1024>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); };
        auto parsed = repsep(elementParserCreator_(), separatorParserCreator_(), initAccumulator, handleElement)(input_);
        expectation_(parsed);
    }

    void test_runtime_accumulator_cvector_() {
        auto parsed = repsep(
            elementParserCreator_(),
            separatorParserCreator_(),
            [] () {return cvector<parser_result_t<ElementParser>, 1024>(); },
            [] (cvector<parser_result_t<ElementParser>, 1024>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); }
        )(input_);
        expectation_(parsed);
    }

    void test_runtime_accumulator_stdvector_() {
        auto parsed = repsep(
            elementParserCreator_(),
            separatorParserCreator_(),
            [] () {return std::vector<parser_result_t<ElementParser>>(); },
            [] (std::vector<parser_result_t<ElementParser>>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); }
        )(input_);
        expectation_(parsed);
    }

    ElementParserCreator elementParserCreator_;
    SeparatorParserCreator separatorParserCreator_;
    Input input_;
    Expectation expectation_;
};
template<class ElementParserCreator, class SeparatorParserCreator, class Expectation>
constexpr auto repsepTestCase_(ElementParserCreator&& elementParserCreator, SeparatorParserCreator&& separatorParserCreator, Input input, Expectation&& expectation) {
    return RepsepTestCase<std::decay_t<ElementParserCreator>, std::decay_t<SeparatorParserCreator>, std::decay_t<Expectation>>(
        std::forward<ElementParserCreator>(elementParserCreator), std::forward<SeparatorParserCreator>(separatorParserCreator), input, std::forward<Expectation>(expectation)
    );
}
template<class ElementParser, class SeparatorParser, class Expectation>
constexpr auto repsepTestCase(ElementParser&& elementParser, SeparatorParser&& separatorParser, Input input, Expectation&& expectation) {
    return repsepTestCase_(
        [elementParser = std::forward<ElementParser>(elementParser)] () {return elementParser; },
        [separatorParser = std::forward<SeparatorParser>(separatorParser)] () {return separatorParser;},
        input,
        std::forward<Expectation>(expectation)
    );
}

namespace test_repsep_empty {
    static_assert(repsepTestCase(string("Elem"), string("Sep"), Input{""}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(0 == parsed.result().size());
        expectTrue_("" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepsepTest, empty) {
    repsepTestCase(string("Elem"), string("Sep"), Input{""}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(0, parsed.result().size());
        EXPECT_EQ("", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep_none {
    static_assert(repsepTestCase(string("Elem"), string("Sep"), Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(0 == parsed.result().size());
        expectTrue_("El" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepsepTest, none) {
    repsepTestCase(string("Elem"), string("Sep"), Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(0, parsed.result().size());
        EXPECT_EQ("El", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep_one {
    static_assert(repsepTestCase(string("Elem"), string("Sep"), Input{"ElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(1 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_("Sa" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepsepTest, one) {
    repsepTestCase(string("Elem"), string("Sep"), Input{"ElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(1, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Sa", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep_one_followingsep {
    static_assert(repsepTestCase(string("Elem"), string("Sep"), Input{"ElemSep"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(1 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_("Sep" == parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_compiletime());
}

TEST(RepsepTest, one_followingsep) {
    repsepTestCase(string("Elem"), string("Sep"), Input{"ElemSep"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(1, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Sep", parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_runtime();
}

namespace test_repsep_two {
    static_assert(repsepTestCase(integer(), string("Sep"), Input{"23Sep39Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(2 == parsed.result().size());
        expectTrue_(parsed.result()[0] == 23);
        expectTrue_(parsed.result()[1] == 39);
        expectTrue_("Text" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepsepTest, two) {
    repsepTestCase(integer(), string("Sep"), Input{"23Sep39Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(2, parsed.result().size());
        EXPECT_EQ(23, parsed.result()[0]);
        EXPECT_EQ(39, parsed.result()[1]);
        EXPECT_EQ("Text", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep_two_followingsep {
    static_assert(repsepTestCase(integer(), string("Sep"), Input{"23Sep39Sep"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(2 == parsed.result().size());
        expectTrue_(parsed.result()[0] == 23);
        expectTrue_(parsed.result()[1] == 39);
        expectTrue_("Sep" == parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_compiletime());
}

TEST(RepsepTest, two_followingsep) {
    repsepTestCase(integer(), string("Sep"), Input{"23Sep39Sep"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(2, parsed.result().size());
        EXPECT_EQ(23, parsed.result()[0]);
        EXPECT_EQ(39, parsed.result()[1]);
        EXPECT_EQ("Sep", parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_runtime();
}

namespace test_repsep_three {
    static_assert(repsepTestCase(integer(), string("Sep"), Input{"23Sep39Sep358Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(3 == parsed.result().size());
        expectTrue_(parsed.result()[0] == 23);
        expectTrue_(parsed.result()[1] == 39);
        expectTrue_(parsed.result()[2] == 358);
        expectTrue_("Text" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepsepTest, three) {
    repsepTestCase(integer(), string("Sep"), Input{"23Sep39Sep358Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(3, parsed.result().size());
        EXPECT_EQ(23, parsed.result()[0]);
        EXPECT_EQ(39, parsed.result()[1]);
        EXPECT_EQ(358, parsed.result()[2]);
        EXPECT_EQ("Text", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep_three_followingsep {
    static_assert(repsepTestCase(integer(), string("Sep"), Input{"23Sep39Sep358SepSomething"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(3 == parsed.result().size());
        expectTrue_(parsed.result()[0] == 23);
        expectTrue_(parsed.result()[1] == 39);
        expectTrue_(parsed.result()[2] == 358);
        expectTrue_("SepSomething" == parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_compiletime());
}

TEST(RepsepTest, three_followingsep) {
    repsepTestCase(integer(), string("Sep"), Input{"23Sep39Sep358SepSomething"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(3, parsed.result().size());
        EXPECT_EQ(23, parsed.result()[0]);
        EXPECT_EQ(39, parsed.result()[1]);
        EXPECT_EQ(358, parsed.result()[2]);
        EXPECT_EQ("SepSomething", parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_runtime();
}

namespace test_repsep_can_convert_subparser_result {
    // note: subparsers return string_view, but the rep1 parser returns string.
    constexpr auto parsed = repsep<cvector<int64_t, 1024>>(digit(), digit())(Input{"526ab"});
    static_assert(std::is_same_v<uint8_t, parser_result_t<decltype(digit())>>);
    static_assert(std::is_same_v<cvector<int64_t, 1024>, std::decay_t<decltype(parsed.result())>>);
    static_assert(parsed.is_success());
    static_assert(2 == parsed.result().size());
    static_assert(5 == parsed.result()[0]);
    static_assert(6 == parsed.result()[1]);
    static_assert("ab" == parsed.next().input);
}

TEST(RepsepTest, canConvertSubparserResult) {
    // note: subparsers return string_view, but the rep parser returns string.
    auto parsed = repsep<std::vector<std::string>>(string("Elem"), string("Sep"))(Input{"ElemSepElemSepEla"});
    static_assert(std::is_same_v<std::vector<std::string>, std::decay_t<decltype(parsed.result())>>);
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(2, parsed.result().size());
    EXPECT_EQ("Elem", parsed.result()[0]);
    EXPECT_EQ("Elem", parsed.result()[1]);
    EXPECT_EQ("SepEla", parsed.next().input);
}

namespace test_repsep_movableonly_none {
    static_assert(repsepTestCase_([] {return movable_only(string("Elem"));}, [] {return movable_only(string("Sep"));}, Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(0 == parsed.result().size());
        expectTrue_("El" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepsepTest, movableonly_none) {
    repsepTestCase_([] {return movable_only(string("Elem"));}, [] {return movable_only(string("Sep"));}, Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(0, parsed.result().size());
        EXPECT_EQ("El", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep_movableonly_two {
    static_assert(repsepTestCase_([] {return movable_only(integer()); }, [] {return movable_only(string("Sep")); }, Input{"23Sep39Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(2 == parsed.result().size());
        expectTrue_(parsed.result()[0] == 23);
        expectTrue_(parsed.result()[1] == 39);
        expectTrue_("Text" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepsepTest, movableonly_two) {
    repsepTestCase_([] {return movable_only(integer());}, [] {return movable_only(string("Sep"));}, Input{"23Sep39Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(2, parsed.result().size());
        EXPECT_EQ(23, parsed.result()[0]);
        EXPECT_EQ(39, parsed.result()[1]);
        EXPECT_EQ("Text", parsed.next().input);
    }).test_all_runtime();
}

TEST(RepsepParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_compiletime) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return repsep(compiletime_optimization, std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"Found,Found"}, ctpc::Input{"Found,FoundBla"}, ctpc::Input{"Found,Found,"}, ctpc::Input{"Found,Found,Bla"}},
            [] () {return string("Found");},
            [] () {return string(",");}
    );
}

TEST(RepsepParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_runtime) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return repsep(runtime_optimization, std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"Found,Found"}, ctpc::Input{"Found,FoundBla"}, ctpc::Input{"Found,Found,"}, ctpc::Input{"Found,Found,Bla"}},
            [] () {return string("Found");},
            [] () {return string(",");}
    );
}

TEST(RepsepParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_customcontainer) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return repsep<cvector<std::string_view, 1024>>(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"Found,Found"}, ctpc::Input{"Found,FoundBla"}, ctpc::Input{"Found,Found,"}, ctpc::Input{"Found,Found,Bla"}},
            [] () {return string("Found");},
            [] () {return string(",");}
    );
}

TEST(RepsepParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_accumulator) {
    static constexpr auto initAccumulator = [] () {return cvector<std::string_view, 1024>(); };
    static constexpr auto handleElement = [] (cvector<std::string_view, 1024>* accumulator, std::string_view element) { accumulator->push_back(element); };
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return repsep(std::forward<decltype(args)>(args)..., initAccumulator, handleElement); },
            {ctpc::Input{""}, ctpc::Input{"Found,Found"}, ctpc::Input{"Found,FoundBla"}, ctpc::Input{"Found,Found,"}, ctpc::Input{"Found,Found,Bla"}},
            [] () {return string("Found");},
            [] () {return string(",");}
    );
}

namespace test_repsep_works_with_movable_only_result_stopafterfullmatch {
    constexpr auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    constexpr auto parsed = repsep(compiletime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aa"});
    static_assert(parsed.is_success());
}

TEST(RepsepParserTest, worksWithMovableOnlyResult_stopafterfullmatch_compiletime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep(compiletime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepsepParserTest, worksWithMovableOnlyResult_stopafterfullmatch_runtime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep(runtime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepsepParserTest, worksWithMovableOnlyResult_stopafterfullmatch_customcontainer) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep<std::vector<movable_only<int>>>(parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepsepParserTest, worksWithMovableOnlyResult_stopafterfullmatch_accumulator) {
    auto initAccumulator = [] () {return cvector<movable_only<int>, 1024>(); };
    auto handleElement = [] (cvector<movable_only<int>, 1024>* accumulator, movable_only<int> element) { accumulator->push_back(std::move(element)); };
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep(parser_with_movable_only_result, parser_with_movable_only_result, initAccumulator, handleElement)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

namespace test_repsep_works_with_movable_only_result_stopafterseparator {
    constexpr auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    constexpr auto parsed = repsep(compiletime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aaa"});
    static_assert(parsed.is_success());
}

TEST(RepsepParserTest, worksWithMovableOnlyResult_stopafterseparator_compiletime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep(compiletime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aaa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepsepParserTest, worksWithMovableOnlyResult_stopafterseparator_runtime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep(runtime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aaa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepsepParserTest, worksWithMovableOnlyResult_stopafterseparator_customcontainer) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep<std::vector<movable_only<int>>>(parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aaa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepsepParserTest, worksWithMovableOnlyResult_stopafterseparator_accumulator) {
    auto initAccumulator = [] () {return cvector<movable_only<int>, 1024>(); };
    auto handleElement = [] (cvector<movable_only<int>, 1024>* accumulator, movable_only<int> element) { accumulator->push_back(std::move(element)); };
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep(parser_with_movable_only_result, parser_with_movable_only_result, initAccumulator, handleElement)(Input{"aaa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepsepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_compiletime) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
          if (input.input.size() > 0) {
              return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
          } else {
              return ParseResult<copy_counting<int>>::failure(input);
          }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep(compiletime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator))(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(4, move_count_pattern);  // two elements, two separators. Each element moved once when parsed, and in the end we move the whole cvector.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(RepsepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_runtime) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep(runtime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator))(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(3, move_count_pattern);  // two elements, two separators. Each element moved once when parsed, the first element moved a second time to dynamically grow the vector
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(RepsepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_runtime_reserving) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep(runtime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator), 2)(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);  // two elements, two separators. Each element moved once when parsed. No dynamic reallocations because we reserve needed size in advance.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(RepsepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_customcontainer) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep<std::vector<copy_counting<int>>>(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator))(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(3, move_count_pattern);  // two elements, two separators. Each element moved once when parsed, the first element moved a second time to dynamically grow the vector
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(RepsepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_customcontainer_reserving) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep<std::vector<copy_counting<int>>>(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator), 2)(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);  // two elements, two separators. Each element moved once when parsed. No dynamic reallocations because we reserve needed size in advance.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(RepsepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_accumulator) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
          if (input.input.size() > 0) {
              return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
          } else {
              return ParseResult<copy_counting<int>>::failure(input);
          }
        };
    };
    auto initAccumulator = [] () -> int {return 0; };
    auto handleElement_byconstref = [] (int* /*accumulator*/, const copy_counting<int>& /*element*/) {};
    auto handleElement_byrvalueref = [] (int* /*accumulator*/, copy_counting<int>&& /*element*/) {};
    auto handleElement_byvalue = [] (int* /*accumulator*/, copy_counting<int> /*element*/) {};

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator), initAccumulator, handleElement_byconstref)(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(0, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);

    parsed = repsep(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator), initAccumulator, handleElement_byrvalueref)(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(0, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);

    parsed = repsep(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator), initAccumulator, handleElement_byvalue)(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

namespace test_repsep1_type {
    // compiletime / runtime overloads
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep1(compiletime_optimization, string("Elem"), string("sep"))(std::declval<Input>()).result())>>, cvector<std::string_view, 1024>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep1(compiletime_optimization, integer(), string("sep"))(std::declval<Input>()).result())>>, cvector<long long, 1024>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep1(runtime_optimization, string("Elem"), string("sep"))(std::declval<Input>()).result())>>, std::vector<std::string_view>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep1(runtime_optimization, integer(), string("sep"))(std::declval<Input>()).result())>>, std::vector<long long>>::value);

    // custom container variant
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep1<std::vector<std::string_view>>(string("Elem"), string("sep"))(std::declval<Input>()).result())>>, std::vector<std::string_view>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep1<std::vector<long long>>(integer(), string("sep"))(std::declval<Input>()).result())>>, std::vector<long long>>::value);

    // custom container variant with parser and container having non-matching but convertible result types   
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep1<std::vector<std::string>>(string("Elem"), string("sep"))(std::declval<Input>()).result())>>, std::vector<std::string>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep1<std::vector<uint8_t>>(integer(), string("sep"))(std::declval<Input>()).result())>>, std::vector<uint8_t>>::value);

    // accumulator variant
    constexpr auto initAccumulator = [] () -> int {return 0; };
    constexpr auto handleElement = [] (int* /*accumulator*/, std::string_view /*element*/) {};
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(repsep1(string("Elem"), string("sep"), initAccumulator, handleElement)(std::declval<Input>()).result())>>, int>::value);
}

template<class ElementParserCreator, class SeparatorParserCreator, class Expectation>
class Repsep1TestCase final {
public:
    using ElementParser = std::decay_t<decltype(std::declval<ElementParserCreator>()())>;
    using SeparatorParser = std::decay_t<decltype(std::declval<SeparatorParserCreator>()())>;

    constexpr Repsep1TestCase(ElementParserCreator elementParserCreator, SeparatorParserCreator separatorParserCreator, Input input, Expectation expectation)
    : elementParserCreator_(std::move(elementParserCreator))
    , separatorParserCreator_(std::move(separatorParserCreator))
    , input_(input)
    , expectation_(std::move(expectation))
    {}

    constexpr bool test_all_compiletime() {
        test_compiletime_();
        test_compiletime_customcontainer_();
        test_compiletime_customcontainer_reservecapacity_();
        test_compiletime_accumulator_();

        return true;
    }

    void test_all_runtime() {
        test_runtime_cvector_();
        test_runtime_stdvector_();
        test_runtime_stdvector_reservecapacity_();
        test_runtime_customcontainer_cvector_();
        test_runtime_customcontainer_cvector_reservecapacity_();
        test_runtime_customcontainer_stdvector_();
        test_runtime_customcontainer_stdvector_reservecapacity_();
        test_runtime_accumulator_cvector_();
        test_runtime_accumulator_stdvector_();
    }

private:

    constexpr void test_compiletime_() {
        auto parsed = repsep1(compiletime_optimization, elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_cvector_() {
        auto parsed = repsep1(compiletime_optimization, elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_stdvector_() {
        auto parsed = repsep1(runtime_optimization, elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_stdvector_reservecapacity_() {
        auto parsed = repsep1(runtime_optimization, elementParserCreator_(), separatorParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_customcontainer_() {
        auto parsed = repsep1<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_customcontainer_reservecapacity_() {
        auto parsed = repsep1<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), separatorParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_cvector_() {
        auto parsed = repsep1<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_stdvector_() {
        auto parsed = repsep1<std::vector<parser_result_t<ElementParser>>>(elementParserCreator_(), separatorParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_cvector_reservecapacity_() {
        auto parsed = repsep1<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), separatorParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_stdvector_reservecapacity_() {
        auto parsed = repsep1<std::vector<parser_result_t<ElementParser>>>(elementParserCreator_(), separatorParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_accumulator_() {
        constexpr auto initAccumulator = [] () {return cvector<parser_result_t<ElementParser>, 1024>(); };
        constexpr auto handleElement = [] (cvector<parser_result_t<ElementParser>, 1024>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); };
        auto parsed = repsep1(elementParserCreator_(), separatorParserCreator_(), initAccumulator, handleElement)(input_);
        expectation_(parsed);
    }

    void test_runtime_accumulator_cvector_() {
        auto parsed = repsep1(
            elementParserCreator_(),
            separatorParserCreator_(),
            [] () {return cvector<parser_result_t<ElementParser>, 1024>(); },
            [] (cvector<parser_result_t<ElementParser>, 1024>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); }
        )(input_);
        expectation_(parsed);
    }

    void test_runtime_accumulator_stdvector_() {
        auto parsed = repsep1(
            elementParserCreator_(),
            separatorParserCreator_(),
            [] () {return std::vector<parser_result_t<ElementParser>>(); },
            [] (std::vector<parser_result_t<ElementParser>>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); }
        )(input_);
        expectation_(parsed);
    }

    ElementParserCreator elementParserCreator_;
    SeparatorParserCreator separatorParserCreator_;
    Input input_;
    Expectation expectation_;
};
template<class ElementParserCreator, class SeparatorParserCreator, class Expectation>
constexpr auto repsep1TestCase_(ElementParserCreator&& elementParserCreator, SeparatorParserCreator&& separatorParserCreator, Input input, Expectation&& expectation) {
    return Repsep1TestCase<std::decay_t<ElementParserCreator>, std::decay_t<SeparatorParserCreator>, std::decay_t<Expectation>>(
        std::forward<ElementParserCreator>(elementParserCreator), std::forward<SeparatorParserCreator>(separatorParserCreator), input, std::forward<Expectation>(expectation)
    );
}
template<class ElementParser, class SeparatorParser, class Expectation>
constexpr auto repsep1TestCase(ElementParser&& elementParser, SeparatorParser&& separatorParser, Input input, Expectation&& expectation) {
    return repsep1TestCase_(
        [elementParser = std::forward<ElementParser>(elementParser)] () {return elementParser; },
        [separatorParser = std::forward<SeparatorParser>(separatorParser)] () {return separatorParser;},
        input,
        std::forward<Expectation>(expectation)
    );
}

namespace test_repsep1_empty {
    static_assert(repsep1TestCase(string("Elem"), string("Sep"), Input{""}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_failure());
        expectTrue_("" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Repsep1Test, empty) {
    repsep1TestCase(string("Elem"), string("Sep"), Input{""}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_failure());
        EXPECT_EQ("", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep1_none {
    static_assert(repsep1TestCase(string("Elem"), string("Sep"), Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_failure());
        expectTrue_("El" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Repsep1Test, none) {
    repsep1TestCase(string("Elem"), string("Sep"), Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_failure());
        EXPECT_EQ("El", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep1_one {
    static_assert(repsep1TestCase(string("Elem"), string("Sep"), Input{"ElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(1 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_("Sa" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Repsep1Test, one) {
    repsep1TestCase(string("Elem"), string("Sep"), Input{"ElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(1, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Sa", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep1_one_followingsep {
    static_assert(repsep1TestCase(string("Elem"), string("Sep"), Input{"ElemSep"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(1 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_("Sep" == parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_compiletime());
}

TEST(Repsep1Test, one_followingsep) {
    repsep1TestCase(string("Elem"), string("Sep"), Input{"ElemSep"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(1, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Sep", parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_runtime();
}

namespace test_repsep1_two {
    static_assert(repsep1TestCase(integer(), string("Sep"), Input{"23Sep39Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(2 == parsed.result().size());
        expectTrue_(parsed.result()[0] == 23);
        expectTrue_(parsed.result()[1] == 39);
        expectTrue_("Text" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Repsep1Test, two) {
    repsep1TestCase(integer(), string("Sep"), Input{"23Sep39Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(2, parsed.result().size());
        EXPECT_EQ(23, parsed.result()[0]);
        EXPECT_EQ(39, parsed.result()[1]);
        EXPECT_EQ("Text", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep1_two_followingsep {
    static_assert(repsep1TestCase(integer(), string("Sep"), Input{"23Sep39Sep"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(2 == parsed.result().size());
        expectTrue_(parsed.result()[0] == 23);
        expectTrue_(parsed.result()[1] == 39);
        expectTrue_("Sep" == parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_compiletime());
}

TEST(Repsep1Test, two_followingsep) {
    repsep1TestCase(integer(), string("Sep"), Input{"23Sep39Sep"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(2, parsed.result().size());
        EXPECT_EQ(23, parsed.result()[0]);
        EXPECT_EQ(39, parsed.result()[1]);
        EXPECT_EQ("Sep", parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_runtime();
}

namespace test_repsep1_three {
    static_assert(repsep1TestCase(integer(), string("Sep"), Input{"23Sep39Sep358Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(3 == parsed.result().size());
        expectTrue_(parsed.result()[0] == 23);
        expectTrue_(parsed.result()[1] == 39);
        expectTrue_(parsed.result()[2] == 358);
        expectTrue_("Text" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Repsep1Test, three) {
    repsep1TestCase(integer(), string("Sep"), Input{"23Sep39Sep358Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(3, parsed.result().size());
        EXPECT_EQ(23, parsed.result()[0]);
        EXPECT_EQ(39, parsed.result()[1]);
        EXPECT_EQ(358, parsed.result()[2]);
        EXPECT_EQ("Text", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep1_three_followingsep {
    static_assert(repsep1TestCase(integer(), string("Sep"), Input{"23Sep39Sep358SepSomething"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(3 == parsed.result().size());
        expectTrue_(parsed.result()[0] == 23);
        expectTrue_(parsed.result()[1] == 39);
        expectTrue_(parsed.result()[2] == 358);
        expectTrue_("SepSomething" == parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_compiletime());
}

TEST(Repsep1Test, three_followingsep) {
    repsep1TestCase(integer(), string("Sep"), Input{"23Sep39Sep358SepSomething"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(3, parsed.result().size());
        EXPECT_EQ(23, parsed.result()[0]);
        EXPECT_EQ(39, parsed.result()[1]);
        EXPECT_EQ(358, parsed.result()[2]);
        EXPECT_EQ("SepSomething", parsed.next().input); // last separator doesn't count, stays in input.
    }).test_all_runtime();
}

namespace test_repsep1_can_convert_subparser_result {
    // note: subparsers return string_view, but the rep1 parser returns string.
    constexpr auto parsed = repsep1<cvector<int64_t, 1024>>(digit(), digit())(Input{"526ab"});
    static_assert(std::is_same_v<uint8_t, parser_result_t<decltype(digit())>>);
    static_assert(std::is_same_v<cvector<int64_t, 1024>, std::decay_t<decltype(parsed.result())>>);
    static_assert(parsed.is_success());
    static_assert(2 == parsed.result().size());
    static_assert(5 == parsed.result()[0]);
    static_assert(6 == parsed.result()[1]);
    static_assert("ab" == parsed.next().input);
}

TEST(Repsep1Test, canConvertSubparserResult) {
    // note: subparsers return string_view, but the rep parser returns string.
    auto parsed = repsep1<std::vector<std::string>>(string("Elem"), string("Sep"))(Input{"ElemSepElemSepEla"});
    static_assert(std::is_same_v<std::vector<std::string>, std::decay_t<decltype(parsed.result())>>);
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(2, parsed.result().size());
    EXPECT_EQ("Elem", parsed.result()[0]);
    EXPECT_EQ("Elem", parsed.result()[1]);
    EXPECT_EQ("SepEla", parsed.next().input);
}

namespace test_repsep1_movableonly_none {
    static_assert(repsep1TestCase_([] {return movable_only(string("Elem"));}, [] {return movable_only(string("Sep"));}, Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_failure());
        expectTrue_("El" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Repsep1Test, movableonly_none) {
    repsep1TestCase_([] {return movable_only(string("Elem"));}, [] {return movable_only(string("Sep"));}, Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_failure());
        EXPECT_EQ("El", parsed.next().input);
    }).test_all_runtime();
}

namespace test_repsep1_movableonly_two {
    static_assert(repsep1TestCase_([] {return movable_only(integer()); }, [] {return movable_only(string("Sep")); }, Input{"23Sep39Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(2 == parsed.result().size());
        expectTrue_(parsed.result()[0] == 23);
        expectTrue_(parsed.result()[1] == 39);
        expectTrue_("Text" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Repsep1Test, movableonly_two) {
    repsep1TestCase_([] {return movable_only(integer());}, [] {return movable_only(string("Sep"));}, Input{"23Sep39Text"}, [] (auto parsed) {
        static_assert(std::is_same_v<int64_t, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(2, parsed.result().size());
        EXPECT_EQ(23, parsed.result()[0]);
        EXPECT_EQ(39, parsed.result()[1]);
        EXPECT_EQ("Text", parsed.next().input);
    }).test_all_runtime();
}

TEST(Repsep1ParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_compiletime) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return repsep1(compiletime_optimization, std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"Found,Found"}, ctpc::Input{"Found,FoundBla"}, ctpc::Input{"Found,Found,"}, ctpc::Input{"Found,Found,Bla"}},
            [] () {return string("Found");},
            [] () {return string(",");}
    );
}

TEST(Repsep1ParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_runtime) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return repsep1(runtime_optimization, std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"Found,Found"}, ctpc::Input{"Found,FoundBla"}, ctpc::Input{"Found,Found,"}, ctpc::Input{"Found,Found,Bla"}},
            [] () {return string("Found");},
            [] () {return string(",");}
    );
}

TEST(Repsep1ParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_customcontainer) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return repsep1<cvector<std::string_view, 1024>>(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"Found,Found"}, ctpc::Input{"Found,FoundBla"}, ctpc::Input{"Found,Found,"}, ctpc::Input{"Found,Found,Bla"}},
            [] () {return string("Found");},
            [] () {return string(",");}
    );
}

TEST(Repsep1ParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_accumulator) {
    static constexpr auto initAccumulator = [] () {return cvector<std::string_view, 1024>(); };
    static constexpr auto handleElement = [] (cvector<std::string_view, 1024>* accumulator, std::string_view element) { accumulator->push_back(element); };
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return repsep1(std::forward<decltype(args)>(args)..., initAccumulator, handleElement); },
            {ctpc::Input{""}, ctpc::Input{"Found,Found"}, ctpc::Input{"Found,FoundBla"}, ctpc::Input{"Found,Found,"}, ctpc::Input{"Found,Found,Bla"}},
            [] () {return string("Found");},
            [] () {return string(",");}
    );
}

namespace test_repsep1_works_with_movable_only_result_stopafterfullmatch {
    constexpr auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    constexpr auto parsed = repsep1(compiletime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aa"});
    static_assert(parsed.is_success());
}

TEST(Repsep1ParserTest, worksWithMovableOnlyResult_stopafterfullmatch_compiletime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep1(compiletime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Repsep1ParserTest, worksWithMovableOnlyResult_stopafterfullmatch_runtime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep1(runtime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Repsep1ParserTest, worksWithMovableOnlyResult_stopafterfullmatch_customcontainer) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep1<std::vector<movable_only<int>>>(parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Repsep1ParserTest, worksWithMovableOnlyResult_stopafterfullmatch_accumulator) {
    auto initAccumulator = [] () {return cvector<movable_only<int>, 1024>(); };
    auto handleElement = [] (cvector<movable_only<int>, 1024>* accumulator, movable_only<int> element) { accumulator->push_back(std::move(element)); };
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep1(parser_with_movable_only_result, parser_with_movable_only_result, initAccumulator, handleElement)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

namespace test_repsep1_works_with_movable_only_result_stopafterseparator {
    constexpr auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    constexpr auto parsed = repsep1(compiletime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aaa"});
    static_assert(parsed.is_success());
}

TEST(Repsep1ParserTest, worksWithMovableOnlyResult_stopafterseparator_compiletime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep1(compiletime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aaa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Repsep1ParserTest, worksWithMovableOnlyResult_stopafterseparator_runtime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep1(runtime_optimization, parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aaa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Repsep1ParserTest, worksWithMovableOnlyResult_stopafterseparator_customcontainer) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep1<std::vector<movable_only<int>>>(parser_with_movable_only_result, parser_with_movable_only_result)(Input{"aaa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Repsep1ParserTest, worksWithMovableOnlyResult_stopafterseparator_accumulator) {
    auto initAccumulator = [] () {return cvector<movable_only<int>, 1024>(); };
    auto handleElement = [] (cvector<movable_only<int>, 1024>* accumulator, movable_only<int> element) { accumulator->push_back(std::move(element)); };
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = repsep1(parser_with_movable_only_result, parser_with_movable_only_result, initAccumulator, handleElement)(Input{"aaa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Repsep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_compiletime) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
          if (input.input.size() > 0) {
              return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
          } else {
              return ParseResult<copy_counting<int>>::failure(input);
          }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep1(compiletime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator))(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(4, move_count_pattern);  // two elements, two separators. Each element moved once when parsed, and in the end we move the whole cvector.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(Repsep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_runtime) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep1(runtime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator))(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(3, move_count_pattern);  // two elements, two separators. Each element moved once when parsed, the first element moved a second time to dynamically grow the vector
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(Repsep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_runtime_reserving) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep1(runtime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator), 2)(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);  // two elements, two separators. Each element moved once when parsed. No dynamic reallocations because we reserve needed size in advance.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(Repsep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_customcontainer) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep1<std::vector<copy_counting<int>>>(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator))(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(3, move_count_pattern);  // two elements, two separators. Each element moved once when parsed, the first element moved a second time to dynamically grow the vector
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(Repsep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_customcontainer_reserving) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep1<std::vector<copy_counting<int>>>(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator), 2)(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);  // two elements, two separators. Each element moved once when parsed. No dynamic reallocations because we reserve needed size in advance.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(Repsep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_accumulator) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
          if (input.input.size() > 0) {
              return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
          } else {
              return ParseResult<copy_counting<int>>::failure(input);
          }
        };
    };
    auto initAccumulator = [] () -> int {return 0; };
    auto handleElement_byconstref = [] (int* /*accumulator*/, const copy_counting<int>& /*element*/) {};
    auto handleElement_byrvalueref = [] (int* /*accumulator*/, copy_counting<int>&& /*element*/) {};
    auto handleElement_byvalue = [] (int* /*accumulator*/, copy_counting<int> /*element*/) {};

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = repsep1(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator), initAccumulator, handleElement_byconstref)(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(0, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);

    parsed = repsep1(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator), initAccumulator, handleElement_byrvalueref)(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(0, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);

    parsed = repsep1(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), parser_with_copycounting_result(&copy_count_separator, &move_count_separator), initAccumulator, handleElement_byvalue)(Input{"aaaa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

namespace test_rep_type {
    // compiletime / runtime overloads
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep(compiletime_optimization, string("Elem"))(std::declval<Input>()).result())>>, cvector<std::string_view, 1024>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep(compiletime_optimization, integer())(std::declval<Input>()).result())>>, cvector<long long, 1024>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep(runtime_optimization, string("Elem"))(std::declval<Input>()).result())>>, std::vector<std::string_view>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep(runtime_optimization, integer())(std::declval<Input>()).result())>>, std::vector<long long>>::value);

    // custom container variant
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep<std::vector<std::string_view>>(string("Elem"))(std::declval<Input>()).result())>>, std::vector<std::string_view>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep<std::vector<long long>>(integer())(std::declval<Input>()).result())>>, std::vector<long long>>::value);

    // custom container variant with parser and container having non-matching but convertible result types   
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep<std::vector<std::string>>(string("Elem"))(std::declval<Input>()).result())>>, std::vector<std::string>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep<std::vector<uint8_t>>(integer())(std::declval<Input>()).result())>>, std::vector<uint8_t>>::value);

    // accumulator variant
    constexpr auto initAccumulator = [] () -> int {return 0; };
    constexpr auto handleElement = [] (int* /*accumulator*/, std::string_view /*element*/) {};
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep(string("Elem"), initAccumulator, handleElement)(std::declval<Input>()).result())>>, int>::value);
}

template<class ElementParserCreator, class Expectation>
class RepTestCase final {
public:
    using ElementParser = std::decay_t<decltype(std::declval<ElementParserCreator>()())>;

    constexpr RepTestCase(ElementParserCreator elementParserCreator, Input input, Expectation expectation)
    : elementParserCreator_(std::move(elementParserCreator))
    , input_(input)
    , expectation_(std::move(expectation))
    {}

    constexpr bool test_all_compiletime() {
        test_compiletime_();
        test_compiletime_customcontainer_();
        test_compiletime_customcontainer_reservecapacity_();
        test_compiletime_accumulator_();

        return true;
    }

    void test_all_runtime() {
        test_runtime_cvector_();
        test_runtime_stdvector_();
        test_runtime_stdvector_reservecapacity_();
        test_runtime_customcontainer_cvector_();
        test_runtime_customcontainer_cvector_reservecapacity_();
        test_runtime_customcontainer_stdvector_();
        test_runtime_customcontainer_stdvector_reservecapacity_();
        test_runtime_accumulator_cvector_();
        test_runtime_accumulator_stdvector_();
    }

private:

    constexpr void test_compiletime_() {
        auto parsed = rep(compiletime_optimization, elementParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_cvector_() {
        auto parsed = rep(compiletime_optimization, elementParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_stdvector_() {
        auto parsed = rep(runtime_optimization, elementParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_stdvector_reservecapacity_() {
        auto parsed = rep(runtime_optimization, elementParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_customcontainer_() {
        auto parsed = rep<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_())(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_customcontainer_reservecapacity_() {
        auto parsed = rep<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_cvector_() {
        auto parsed = rep<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_stdvector_() {
        auto parsed = rep<std::vector<parser_result_t<ElementParser>>>(elementParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_cvector_reservecapacity_() {
        auto parsed = rep<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_stdvector_reservecapacity_() {
        auto parsed = rep<std::vector<parser_result_t<ElementParser>>>(elementParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_accumulator_() {
        constexpr auto initAccumulator = [] () {return cvector<parser_result_t<ElementParser>, 1024>(); };
        constexpr auto handleElement = [] (cvector<parser_result_t<ElementParser>, 1024>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); };
        auto parsed = rep(elementParserCreator_(), initAccumulator, handleElement)(input_);
        expectation_(parsed);
    }

    void test_runtime_accumulator_cvector_() {
        auto parsed = rep(
            elementParserCreator_(),
            [] () {return cvector<parser_result_t<ElementParser>, 1024>(); },
            [] (cvector<parser_result_t<ElementParser>, 1024>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); }
        )(input_);
        expectation_(parsed);
    }

    void test_runtime_accumulator_stdvector_() {
        auto parsed = rep(
            elementParserCreator_(),
            [] () {return std::vector<parser_result_t<ElementParser>>(); },
            [] (std::vector<parser_result_t<ElementParser>>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); }
        )(input_);
        expectation_(parsed);
    }

    ElementParserCreator elementParserCreator_;
    Input input_;
    Expectation expectation_;
};
template<class ElementParserCreator, class Expectation>
constexpr auto repTestCase_(ElementParserCreator&& elementParserCreator, Input input, Expectation&& expectation) {
    return RepTestCase<std::decay_t<ElementParserCreator>, std::decay_t<Expectation>>(
        std::move(elementParserCreator), input, std::forward<Expectation>(expectation)
    );
}
template<class ElementParser, class Expectation>
constexpr auto repTestCase(ElementParser&& elementParser, Input input, Expectation&& expectation) {
    return repTestCase_(
        [elementParser = std::forward<ElementParser>(elementParser)] () {return elementParser; },
        input,
        std::forward<Expectation>(expectation)
    );
}

namespace test_rep_empty {
    static_assert(repTestCase(string("Elem"), Input{""}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(0 == parsed.result().size());
        expectTrue_("" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepParserTest, empty) {
    repTestCase(string("Elem"), Input{""}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(0, parsed.result().size());
        EXPECT_EQ("", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep_none {
    static_assert(repTestCase(string("Elem"), Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(0 == parsed.result().size());
        expectTrue_("El" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepParserTest, none) {
    repTestCase(string("Elem"), Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(0, parsed.result().size());
        EXPECT_EQ("El", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep_one {
    static_assert(repTestCase(string("Elem"), Input{"ElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(1 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_("Sa" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepParserTest, one) {
    repTestCase(string("Elem"), Input{"ElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(1, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Sa", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep_two {
    static_assert(repTestCase(string("Elem"), Input{"ElemElem"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(2 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_(parsed.result()[1] == "Elem");
        expectTrue_("" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepParserTest, two) {
    repTestCase(string("Elem"), Input{"ElemElem"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(2, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Elem", parsed.result()[1]);
        EXPECT_EQ("", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep_three {
    static_assert(repTestCase(string("Elem"), Input{"ElemElemElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(3 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_(parsed.result()[1] == "Elem");
        expectTrue_(parsed.result()[2] == "Elem");
        expectTrue_("Sa" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepParserTest, three) {
    repTestCase(string("Elem"), Input{"ElemElemElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(3, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Elem", parsed.result()[1]);
        EXPECT_EQ("Elem", parsed.result()[2]);
        EXPECT_EQ("Sa", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep_can_convert_subparser_result {
    // note: subparsers return string_view, but the rep1 parser returns string.
    constexpr auto parsed = rep<cvector<int64_t, 1024>>(digit())(Input{"526ab"});
    static_assert(std::is_same_v<uint8_t, parser_result_t<decltype(digit())>>);
    static_assert(std::is_same_v<cvector<int64_t, 1024>, std::decay_t<decltype(parsed.result())>>);
    static_assert(parsed.is_success());
    static_assert(3 == parsed.result().size());
    static_assert(5 == parsed.result()[0]);
    static_assert(2 == parsed.result()[1]);
    static_assert(6 == parsed.result()[2]);
    static_assert("ab" == parsed.next().input);
}

TEST(RepParserTest, canConvertSubparserResult) {
    // note: subparsers return string_view, but the rep parser returns string.
    auto parsed = rep<std::vector<std::string>>(string("Elem"))(Input{"ElemElemSepEla"});
    static_assert(std::is_same_v<std::vector<std::string>, std::decay_t<decltype(parsed.result())>>);
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(2, parsed.result().size());
    EXPECT_EQ("Elem", parsed.result()[0]);
    EXPECT_EQ("Elem", parsed.result()[1]);
    EXPECT_EQ("SepEla", parsed.next().input);
}

namespace test_rep_movableonly_none {
    static_assert(repTestCase_([] {return movable_only(string("Elem"));} , Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(0 == parsed.result().size());
        expectTrue_("El" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepParserTest, movableonly_none) {
    repTestCase_([] {return movable_only(string("Elem"));}, Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(0, parsed.result().size());
        EXPECT_EQ("El", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep_movableonly_two {
    static_assert(repTestCase_([] {return movable_only(string("Elem"));}, Input{"ElemElem"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(2 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_(parsed.result()[1] == "Elem");
        expectTrue_("" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(RepParserTest, movableonly_two) {
    repTestCase_([] {return movable_only(string("Elem"));}, Input{"ElemElem"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(2, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Elem", parsed.result()[1]);
        EXPECT_EQ("", parsed.next().input);
    }).test_all_runtime();
}

TEST(RepParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_compiletime) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return rep(compiletime_optimization, std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundFound"}, ctpc::Input{"FoundFoundBla"}, ctpc::Input{"FoundFound,"}, ctpc::Input{"FoundFoundBla"}},
            [] () {return string("Found");}
    );
}

TEST(RepParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_runtime) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return rep(runtime_optimization, std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundFound"}, ctpc::Input{"FoundFoundBla"}, ctpc::Input{"FoundFound,"}, ctpc::Input{"FoundFoundBla"}},
            [] () {return string("Found");}
    );
}

TEST(RepParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_customcontainer) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return rep<cvector<std::string_view, 1024>>(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundFound"}, ctpc::Input{"FoundFoundBla"}, ctpc::Input{"FoundFound,"}, ctpc::Input{"FoundFoundBla"}},
            [] () {return string("Found");}
    );
}

TEST(RepParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_accumulator) {
    static constexpr auto initAccumulator = [] () {return cvector<std::string_view, 1024>(); };
    static constexpr auto handleElement = [] (cvector<std::string_view, 1024>* accumulator, std::string_view element) { accumulator->push_back(element); };
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return rep(std::forward<decltype(args)>(args)..., initAccumulator, handleElement); },
            {ctpc::Input{""}, ctpc::Input{"FoundFound"}, ctpc::Input{"FoundFoundBla"}, ctpc::Input{"FoundFound,"}, ctpc::Input{"FoundFoundBla"}},
            [] () {return string("Found");}
    );
}

namespace test_rep_works_with_movable_only_result {
    constexpr auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    constexpr auto parsed = rep(compiletime_optimization, parser_with_movable_only_result)(Input{"aa"});
    static_assert(parsed.is_success());
}

TEST(RepParserTest, worksWithMovableOnlyResult_compiletime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = rep(compiletime_optimization, parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepParserTest, worksWithMovableOnlyResult_runtime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = rep(runtime_optimization, parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepParserTest, worksWithMovableOnlyResult_customcontainer) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = rep<std::vector<movable_only<int>>>(parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepParserTest, worksWithMovableOnlyResult_accumulator) {
    auto initAccumulator = [] () {return cvector<movable_only<int>, 1024>(); };
    auto handleElement = [] (cvector<movable_only<int>, 1024>* accumulator, movable_only<int> element) { accumulator->push_back(std::move(element)); };
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = rep(parser_with_movable_only_result, initAccumulator, handleElement)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(RepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_compiletime) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
          if (input.input.size() > 0) {
              return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
          } else {
              return ParseResult<copy_counting<int>>::failure(input);
          }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep(compiletime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern))(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(4, move_count_pattern);  // two elements. Each element moved once when parsed, and in the end we move the whole cvector.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(RepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_runtime) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep(runtime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern))(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(3, move_count_pattern);  // two elements. Each element moved once when parsed, the first element moved a second time to dynamically grow the vector
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(RepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_runtime_reserving) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep(runtime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), 2)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);  // two elements. Each element moved once when parsed. No dynamic reallocations because we reserve needed size in advance.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(RepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_customcontainer) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep<std::vector<copy_counting<int>>>(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern))(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(3, move_count_pattern);  // two elements. Each element moved once when parsed, the first element moved a second time to dynamically grow the vector
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(RepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_customcontainer_reserving) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep<std::vector<copy_counting<int>>>(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), 2)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);  // two elements. Each element moved once when parsed. No dynamic reallocations because we reserve needed size in advance.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(RepParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_accumulator) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
          if (input.input.size() > 0) {
              return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
          } else {
              return ParseResult<copy_counting<int>>::failure(input);
          }
        };
    };
    auto initAccumulator = [] () -> int {return 0; };
    auto handleElement_byconstref = [] (int* /*accumulator*/, const copy_counting<int>& /*element*/) {};
    auto handleElement_byrvalueref = [] (int* /*accumulator*/, copy_counting<int>&& /*element*/) {};
    auto handleElement_byvalue = [] (int* /*accumulator*/, copy_counting<int> /*element*/) {};

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), initAccumulator, handleElement_byconstref)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(0, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);

    parsed = rep(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), initAccumulator, handleElement_byrvalueref)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(0, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);

    parsed = rep(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), initAccumulator, handleElement_byvalue)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

namespace test_rep1_type {
    // compiletime / runtime overloads
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep1(compiletime_optimization, string("Elem"))(std::declval<Input>()).result())>>, cvector<std::string_view, 1024>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep1(compiletime_optimization, integer())(std::declval<Input>()).result())>>, cvector<long long, 1024>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep1(runtime_optimization, string("Elem"))(std::declval<Input>()).result())>>, std::vector<std::string_view>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep1(runtime_optimization, integer())(std::declval<Input>()).result())>>, std::vector<long long>>::value);

    // custom container variant
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep1<std::vector<std::string_view>>(string("Elem"))(std::declval<Input>()).result())>>, std::vector<std::string_view>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep1<std::vector<long long>>(integer())(std::declval<Input>()).result())>>, std::vector<long long>>::value);

    // custom container variant with parser and container having non-matching but convertible result types   
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep1<std::vector<std::string>>(string("Elem"))(std::declval<Input>()).result())>>, std::vector<std::string>>::value);
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep1<std::vector<uint8_t>>(integer())(std::declval<Input>()).result())>>, std::vector<uint8_t>>::value);

    // accumulator variant
    constexpr auto initAccumulator = [] () -> int {return 0; };
    constexpr auto handleElement = [] (int* /*accumulator*/, std::string_view /*element*/) {};
    static_assert(std::is_same<std::remove_cv_t<std::remove_reference_t<decltype(rep1(string("Elem"), initAccumulator, handleElement)(std::declval<Input>()).result())>>, int>::value);
}
template<class ElementParserCreator, class Expectation>
class Rep1TestCase final {
public:
    using ElementParser = std::decay_t<decltype(std::declval<ElementParserCreator>()())>;

    constexpr Rep1TestCase(ElementParserCreator elementParserCreator, Input input, Expectation expectation)
    : elementParserCreator_(std::move(elementParserCreator))
    , input_(input)
    , expectation_(std::move(expectation))
    {}

    constexpr bool test_all_compiletime() {
        test_compiletime_();
        test_compiletime_customcontainer_();
        test_compiletime_customcontainer_reservecapacity_();
        test_compiletime_accumulator_();

        return true;
    }

    void test_all_runtime() {
        test_runtime_cvector_();
        test_runtime_stdvector_();
        test_runtime_stdvector_reservecapacity_();
        test_runtime_customcontainer_cvector_();
        test_runtime_customcontainer_cvector_reservecapacity_();
        test_runtime_customcontainer_stdvector_();
        test_runtime_customcontainer_stdvector_reservecapacity_();
        test_runtime_accumulator_cvector_();
        test_runtime_accumulator_stdvector_();
    }

private:

    constexpr void test_compiletime_() {
        auto parsed = rep1(compiletime_optimization, elementParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_cvector_() {
        auto parsed = rep1(compiletime_optimization, elementParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_stdvector_() {
        auto parsed = rep1(runtime_optimization, elementParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_stdvector_reservecapacity_() {
        auto parsed = rep1(runtime_optimization, elementParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_customcontainer_() {
        auto parsed = rep1<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_())(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_customcontainer_reservecapacity_() {
        auto parsed = rep1<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_cvector_() {
        auto parsed = rep1<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_stdvector_() {
        auto parsed = rep1<std::vector<parser_result_t<ElementParser>>>(elementParserCreator_())(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_cvector_reservecapacity_() {
        auto parsed = rep1<cvector<parser_result_t<ElementParser>, 1024>>(elementParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    void test_runtime_customcontainer_stdvector_reservecapacity_() {
        auto parsed = rep1<std::vector<parser_result_t<ElementParser>>>(elementParserCreator_(), 100)(input_);
        expectation_(parsed);
    }

    constexpr void test_compiletime_accumulator_() {
        constexpr auto initAccumulator = [] () {return cvector<parser_result_t<ElementParser>, 1024>(); };
        constexpr auto handleElement = [] (cvector<parser_result_t<ElementParser>, 1024>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); };
        auto parsed = rep1(elementParserCreator_(), initAccumulator, handleElement)(input_);
        expectation_(parsed);
    }

    void test_runtime_accumulator_cvector_() {
        auto parsed = rep1(
            elementParserCreator_(),
            [] () {return cvector<parser_result_t<ElementParser>, 1024>(); },
            [] (cvector<parser_result_t<ElementParser>, 1024>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); }
        )(input_);
        expectation_(parsed);
    }

    void test_runtime_accumulator_stdvector_() {
        auto parsed = rep1(
            elementParserCreator_(),
            [] () {return std::vector<parser_result_t<ElementParser>>(); },
            [] (std::vector<parser_result_t<ElementParser>>* accumulator, parser_result_t<ElementParser> element) { accumulator->push_back(element); }
        )(input_);
        expectation_(parsed);
    }

    ElementParserCreator elementParserCreator_;
    Input input_;
    Expectation expectation_;
};
template<class ElementParserCreator, class Expectation>
constexpr auto rep1TestCase_(ElementParserCreator&& elementParserCreator, Input input, Expectation&& expectation) {
    return Rep1TestCase<std::decay_t<ElementParserCreator>, std::decay_t<Expectation>>(
        std::move(elementParserCreator), input, std::forward<Expectation>(expectation)
    );
}
template<class ElementParser, class Expectation>
constexpr auto rep1TestCase(ElementParser&& elementParser, Input input, Expectation&& expectation) {
    return rep1TestCase_(
        [elementParser = std::forward<ElementParser>(elementParser)] () {return elementParser; },
        input,
        std::forward<Expectation>(expectation)
    );
}

namespace test_rep1_empty {
    static_assert(rep1TestCase(string("Elem"), Input{""}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_failure());
        expectTrue_("" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Rep1ParserTest, empty) {
    rep1TestCase(string("Elem"), Input{""}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_failure());
        EXPECT_EQ("", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep1_none {
    static_assert(rep1TestCase(string("Elem"), Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_failure());
        expectTrue_("El" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Rep1ParserTest, none) {
    rep1TestCase(string("Elem"), Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_failure());
        EXPECT_EQ("El", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep1_one {
    static_assert(rep1TestCase(string("Elem"), Input{"ElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(1 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_("Sa" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Rep1ParserTest, one) {
    rep1TestCase(string("Elem"), Input{"ElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(1, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Sa", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep1_two {
    static_assert(rep1TestCase(string("Elem"), Input{"ElemElem"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(2 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_(parsed.result()[1] == "Elem");
        expectTrue_("" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Rep1ParserTest, two) {
    rep1TestCase(string("Elem"), Input{"ElemElem"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(2, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Elem", parsed.result()[1]);
        EXPECT_EQ("", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep1_three {
    static_assert(rep1TestCase(string("Elem"), Input{"ElemElemElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(3 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_(parsed.result()[1] == "Elem");
        expectTrue_(parsed.result()[2] == "Elem");
        expectTrue_("Sa" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Rep1ParserTest, three) {
    rep1TestCase(string("Elem"), Input{"ElemElemElemSa"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(3, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Elem", parsed.result()[1]);
        EXPECT_EQ("Elem", parsed.result()[2]);
        EXPECT_EQ("Sa", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep1_can_convert_subparser_result {
    // note: subparsers return string_view, but the rep1 parser returns string.
    constexpr auto parsed = rep1<cvector<int64_t, 1024>>(digit())(Input{"526ab"});
    static_assert(std::is_same_v<uint8_t, parser_result_t<decltype(digit())>>);
    static_assert(std::is_same_v<cvector<int64_t, 1024>, std::decay_t<decltype(parsed.result())>>);
    static_assert(parsed.is_success());
    static_assert(3 == parsed.result().size());
    static_assert(5 == parsed.result()[0]);
    static_assert(2 == parsed.result()[1]);
    static_assert(6 == parsed.result()[2]);
    static_assert("ab" == parsed.next().input);
}

TEST(Rep1ParserTest, canConvertSubparserResult) {
    // note: subparsers return string_view, but the rep1 parser returns string.
    auto parsed = rep1<std::vector<std::string>>(string("Elem"))(Input{"ElemElemSepEla"});
    static_assert(std::is_same_v<std::vector<std::string>, std::decay_t<decltype(parsed.result())>>);
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(2, parsed.result().size());
    EXPECT_EQ("Elem", parsed.result()[0]);
    EXPECT_EQ("Elem", parsed.result()[1]);
    EXPECT_EQ("SepEla", parsed.next().input);
}

namespace test_rep1_movableonly_none {
    static_assert(rep1TestCase_([] {return movable_only(string("Elem"));} , Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_failure());
        expectTrue_("El" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Rep1ParserTest, movableonly_none) {
    rep1TestCase_([] {return movable_only(string("Elem"));}, Input{"El"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_failure());
        EXPECT_EQ("El", parsed.next().input);
    }).test_all_runtime();
}

namespace test_rep1_movableonly_two {
    static_assert(rep1TestCase_([] {return movable_only(string("Elem"));}, Input{"ElemElem"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        expectTrue_(parsed.is_success());
        expectTrue_(2 == parsed.result().size());
        expectTrue_(parsed.result()[0] == "Elem");
        expectTrue_(parsed.result()[1] == "Elem");
        expectTrue_("" == parsed.next().input);
    }).test_all_compiletime());
}

TEST(Rep1ParserTest, movableonly_two) {
    rep1TestCase_([] {return movable_only(string("Elem"));}, Input{"ElemElem"}, [] (auto parsed) {
        static_assert(std::is_same_v<std::string_view, typename decltype(parsed)::result_type::value_type>);
        EXPECT_TRUE(parsed.is_success());
        EXPECT_EQ(2, parsed.result().size());
        EXPECT_EQ("Elem", parsed.result()[0]);
        EXPECT_EQ("Elem", parsed.result()[1]);
        EXPECT_EQ("", parsed.next().input);
    }).test_all_runtime();
}

TEST(Rep1ParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_compiletime) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return rep1(compiletime_optimization, std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundFound"}, ctpc::Input{"FoundFoundBla"}, ctpc::Input{"FoundFound,"}, ctpc::Input{"FoundFoundBla"}},
            [] () {return string("Found");}
    );
}

TEST(Rep1ParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_runtime) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return rep1(runtime_optimization, std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundFound"}, ctpc::Input{"FoundFoundBla"}, ctpc::Input{"FoundFound,"}, ctpc::Input{"FoundFoundBla"}},
            [] () {return string("Found");}
    );
}

TEST(Rep1ParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_customcontainer) {
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return rep1<cvector<std::string_view, 1024>>(std::forward<decltype(args)>(args)...); },
            {ctpc::Input{""}, ctpc::Input{"FoundFound"}, ctpc::Input{"FoundFoundBla"}, ctpc::Input{"FoundFound,"}, ctpc::Input{"FoundFoundBla"}},
            [] () {return string("Found");}
    );
}

TEST(Rep1ParserTest, doesntCopyOrMoveParsersMoreThanAbsolutelyNecessary_accumulator) {
    static constexpr auto initAccumulator = [] () {return cvector<std::string_view, 1024>(); };
    static constexpr auto handleElement = [] (cvector<std::string_view, 1024>* accumulator, std::string_view element) { accumulator->push_back(element); };
    testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(
            [] (auto&&... args) {return rep1(std::forward<decltype(args)>(args)..., initAccumulator, handleElement); },
            {ctpc::Input{""}, ctpc::Input{"FoundFound"}, ctpc::Input{"FoundFoundBla"}, ctpc::Input{"FoundFound,"}, ctpc::Input{"FoundFoundBla"}},
            [] () {return string("Found");}
    );
}

namespace test_rep1_works_with_movable_only_result {
    constexpr auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    constexpr auto parsed = rep1(compiletime_optimization, parser_with_movable_only_result)(Input{"aa"});
    static_assert(parsed.is_success());
}

TEST(Rep1ParserTest, worksWithMovableOnlyResult_compiletime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = rep1(compiletime_optimization, parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Rep1ParserTest, worksWithMovableOnlyResult_runtime) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = rep1(runtime_optimization, parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Rep1ParserTest, worksWithMovableOnlyResult_customcontainer) {
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = rep1<std::vector<movable_only<int>>>(parser_with_movable_only_result)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Rep1ParserTest, worksWithMovableOnlyResult_accumulator) {
    auto initAccumulator = [] () {return cvector<movable_only<int>, 1024>(); };
    auto handleElement = [] (cvector<movable_only<int>, 1024>* accumulator, movable_only<int> element) { accumulator->push_back(std::move(element)); };
    auto parser_with_movable_only_result = map(elem('a'), [] (auto) {return movable_only<int>(3);});
    auto parsed = rep1(parser_with_movable_only_result, initAccumulator, handleElement)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
}

TEST(Rep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_compiletime) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
          if (input.input.size() > 0) {
              return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
          } else {
              return ParseResult<copy_counting<int>>::failure(input);
          }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep1(compiletime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern))(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(4, move_count_pattern);  // two elements. Each element moved once when parsed, and in the end we move the whole cvector.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(Rep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_runtime) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep1(runtime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern))(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(3, move_count_pattern);  // two elements. Each element moved once when parsed, the first element moved a second time to dynamically grow the vector
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(Rep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_runtime_reserving) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep1(runtime_optimization, parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), 2)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);  // two elements. Each element moved once when parsed. No dynamic reallocations because we reserve needed size in advance.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(Rep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_customcontainer) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep1<std::vector<copy_counting<int>>>(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern))(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(3, move_count_pattern);  // two elements. Each element moved once when parsed, the first element moved a second time to dynamically grow the vector
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(Rep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_customcontainer_reserving) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
            if (input.input.size() > 0) {
                return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
            } else {
                return ParseResult<copy_counting<int>>::failure(input);
            }
        };
    };

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep1<std::vector<copy_counting<int>>>(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), 2)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);  // two elements. Each element moved once when parsed. No dynamic reallocations because we reserve needed size in advance.
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

TEST(Rep1ParserTest, doesntCopyOrMoveResultMoreThanAbsolutelyNecessary_accumulator) {
    constexpr auto parser_with_copycounting_result = [] (size_t* copy_count, size_t* move_count) {
        return [=] (Input input) {
          if (input.input.size() > 0) {
              return ParseResult<copy_counting<int>>::success(Input{input.input.substr(1)}, 3, copy_count, move_count);
          } else {
              return ParseResult<copy_counting<int>>::failure(input);
          }
        };
    };
    auto initAccumulator = [] () -> int {return 0; };
    auto handleElement_byconstref = [] (int* /*accumulator*/, const copy_counting<int>& /*element*/) {};
    auto handleElement_byrvalueref = [] (int* /*accumulator*/, copy_counting<int>&& /*element*/) {};
    auto handleElement_byvalue = [] (int* /*accumulator*/, copy_counting<int> /*element*/) {};

    size_t copy_count_pattern = 0, move_count_pattern = 0, copy_count_separator = 0, move_count_separator = 0;

    auto parsed = rep1(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), initAccumulator, handleElement_byconstref)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(0, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);

    parsed = rep1(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), initAccumulator, handleElement_byrvalueref)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(0, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);

    parsed = rep1(parser_with_copycounting_result(&copy_count_pattern, &move_count_pattern), initAccumulator, handleElement_byvalue)(Input{"aa"});
    EXPECT_TRUE(parsed.is_success());
    EXPECT_EQ(0, copy_count_pattern);
    EXPECT_EQ(2, move_count_pattern);
    EXPECT_EQ(0, copy_count_separator);
    EXPECT_EQ(0, move_count_separator);
}

}
