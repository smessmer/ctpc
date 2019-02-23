#pragma once

#include "parsers/parse_result.h"
#include <gtest/gtest.h>
#include <array>

template<class Parser>
class movable_only final {
public:
    // TODO Remove default constructor once not needed by cvector anymore
    constexpr explicit movable_only(): _parser() {}

    constexpr explicit movable_only(Parser parser)
    : _parser(std::move(parser)) {}

    template<class... Args>
    constexpr auto operator()(Args&&... args) const {
        return _parser(std::forward<Args>(args)...);
    }

    constexpr movable_only(const movable_only&) = delete;
    constexpr movable_only& operator=(const movable_only&) = delete;
    constexpr movable_only(movable_only&&) noexcept = default;
    constexpr movable_only& operator=(movable_only&&) noexcept = default;

private:
    Parser _parser;
};

template<class Parser>
class copy_counting final {
public:
    // TODO Remove default constructor once not needed by cvector anymore
    constexpr explicit copy_counting(): _copy_counter(nullptr), _move_counter(nullptr), _parser() {}

    constexpr explicit copy_counting(Parser parser, size_t* copy_counter, size_t* move_counter)
    : _copy_counter(copy_counter), _move_counter(move_counter), _parser(std::move(parser)) {}

    template<class... Args>
    constexpr auto operator()(Args&&... args) const {
        return _parser(std::forward<Args>(args)...);
    }

    constexpr copy_counting(const copy_counting& rhs)
    : _copy_counter(rhs._copy_counter), _move_counter(rhs._move_counter), _parser(rhs._parser) {
        if (nullptr != _copy_counter) {
            ++*_copy_counter;
        }
    }

    constexpr copy_counting& operator=(const copy_counting& rhs) {
        _copy_counter = rhs._copy_counter;
        _move_counter = rhs._move_counter;
        _parser = rhs._parser;
        if (nullptr != _copy_counter) {
            ++*_copy_counter;
        }
        return *this;
    }

    constexpr copy_counting(copy_counting&& rhs) noexcept
    : _copy_counter(rhs._copy_counter), _move_counter(rhs._move_counter), _parser(std::move(rhs._parser)) {
        if (nullptr != _move_counter) {
            ++*_move_counter;
        }
    }

    constexpr copy_counting& operator=(copy_counting&& rhs) noexcept {
        _copy_counter = rhs._copy_counter;
        _move_counter = rhs._move_counter;
        _parser = std::move(rhs._parser);
        if (nullptr != _move_counter) {
            ++*_move_counter;
        }
        return *this;
    }

private:
    size_t* _copy_counter;
    size_t* _move_counter;
    Parser _parser;
};


template<class ParentParserCreator, class... ChildParserCreators>
class TestDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary final {
public:
    explicit TestDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(ParentParserCreator parentCreator, std::vector<ctpc::Input> callingInputs, ChildParserCreators... childCreators)
    : _parentCreator(std::move(parentCreator)), _childCreators(std::make_tuple(std::move(childCreators)...)), _callingInputs(std::move(callingInputs)) {}

    void runTest() {
        _testConstructionFromTemporaryOnlyMovesOnce(index_sequence);
        _testConstructionFromRValueOnlyMovesOnce(index_sequence);
        _testConstructionFromLValueOnlyCopiesOnce(index_sequence);
        _testCopyingParentOnlyCopiesChildrenOnce(index_sequence);
        _testMovingParentOnlyMovesChildrenOnce(index_sequence);
        _testCallingDoesntCopyOrMove(index_sequence);
    }

private:
    struct Counts final {
        std::array<size_t, sizeof...(ChildParserCreators)> copy;
        std::array<size_t, sizeof...(ChildParserCreators)> move;

        explicit Counts() {
            reset();
        }

        void reset() {
            for (size_t& count : copy) {
                count = 0;
            }
            for (size_t& count : move) {
                count = 0;
            }
        }

        void expect(size_t expected_copy_count, size_t expected_move_count) {
            for (size_t count : copy) {
                EXPECT_EQ(expected_copy_count, count);
            }
            for (size_t count : move) {
                EXPECT_EQ(expected_move_count, count);
            }
        }
    };

    template<size_t... Indices>
    void _testConstructionFromTemporaryOnlyMovesOnce(std::index_sequence<Indices...>) {
        Counts counts;
        auto parser = _parentCreator(copy_counting(std::get<Indices>(_childCreators)(), &std::get<Indices>(counts.copy), &std::get<Indices>(counts.move))...);
        counts.expect(0, 1);

        (void)(parser); // fix unused parameter warning
    }

    template<size_t... Indices>
    void _testConstructionFromRValueOnlyMovesOnce(std::index_sequence<Indices...>) {
        Counts counts;
        auto child_parsers = std::make_tuple(copy_counting(std::get<Indices>(_childCreators)(), &std::get<Indices>(counts.copy), &std::get<Indices>(counts.move))...);

        counts.reset();
        auto parser = _parentCreator(std::move(std::get<Indices>(child_parsers))...);
        counts.expect(0, 1);

        (void)(parser); // fix unused parameter warning
    }

    template<size_t... Indices>
    void _testConstructionFromLValueOnlyCopiesOnce(std::index_sequence<Indices...>) {
        Counts counts;
        auto child_parsers = std::make_tuple(copy_counting(std::get<Indices>(_childCreators)(), &std::get<Indices>(counts.copy), &std::get<Indices>(counts.move))...);

        counts.reset();
        auto parser = _parentCreator(std::get<Indices>(child_parsers)...);
        counts.expect(1, 0);

        (void)(parser); // fix unused parameter warning
    }

    template<size_t... Indices>
    void _testCopyingParentOnlyCopiesChildrenOnce(std::index_sequence<Indices...>) {
        Counts counts;
        auto parser = _parentCreator(copy_counting(std::get<Indices>(_childCreators)(), &std::get<Indices>(counts.copy), &std::get<Indices>(counts.move))...);

        counts.reset();
        auto copy = parser;
        counts.expect(1, 0);

        (void)(copy); // fix unused parameter warning
    }

    template<size_t... Indices>
    void _testMovingParentOnlyMovesChildrenOnce(std::index_sequence<Indices...>) {
        Counts counts;
        auto parser = _parentCreator(copy_counting(std::get<Indices>(_childCreators)(), &std::get<Indices>(counts.copy), &std::get<Indices>(counts.move))...);

        counts.reset();
        auto moved = std::move(parser);
        counts.expect(0, 1);

        (void)(moved); // fix unused parameter warning
    }

    template<size_t... Indices>
    void _testCallingDoesntCopyOrMove(std::index_sequence<Indices...>) {
        Counts counts;
        auto parser = _parentCreator(copy_counting(std::get<Indices>(_childCreators)(), &std::get<Indices>(counts.copy), &std::get<Indices>(counts.move))...);
        counts.reset();

        for (auto input : _callingInputs) {
            parser(input);
        }
        counts.expect(0, 0);
    }

    ParentParserCreator _parentCreator;
    std::tuple<ChildParserCreators...> _childCreators;
    std::vector<ctpc::Input> _callingInputs;
    static constexpr auto index_sequence = std::make_index_sequence<sizeof...(ChildParserCreators)>();
};

template<class ParentParserCreator, class... ChildParserCreators>
inline void testDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary(ParentParserCreator parentCreator, std::vector<ctpc::Input> callingInputs, ChildParserCreators... childCreators) {
    TestDoesntCopyOrMoveParsersMoreThanAbsolutelyNecessary<ParentParserCreator, ChildParserCreators...>(
        std::move(parentCreator), std::move(callingInputs), std::move(childCreators)...
    ).runTest();
}

constexpr auto failure_parser_with_movableonly_result() {
    return [] (ctpc::Input input) {
        return ctpc::ParseResult<movable_only<int>>::failure(input);
    };
}

constexpr auto success_parser_with_movableonly_result() {
    return [] (ctpc::Input input) {
        return ctpc::ParseResult<movable_only<int>>::success(input, movable_only<int>(3));
    };
}

constexpr auto success_parser_with_copycounting_result(size_t* copy_count, size_t* move_count) {
    return [copy_count, move_count] (ctpc::Input input) {
        return ctpc::ParseResult<copy_counting<int>>::success(input, 3, copy_count, move_count);
    };
}

constexpr auto failure_parser_with_copycounting_result() {
    return [] (ctpc::Input input) {
        return ctpc::ParseResult<copy_counting<int>>::failure(input);
    };
}
