#pragma once

#include "parsers/parse_result.h"
#include "parsers/basic_parsers.h"
#include "parsers/map.h"
#include "parsers/utils/cvector.h"

// TODO Test cvector-returning parsers to work with types that don't have a default constructor after cvector is fixed


namespace ctpc {

    namespace details {
        template<class ElementParser, size_t MAX_SIZE>
        using CompiletimeContainerForParser = cvector<parser_result_t<ElementParser>, MAX_SIZE>;
        template<class ElementParser>
        using RuntimeContainerForParser = std::vector<parser_result_t<ElementParser>>;

        template<bool NoMatchIsOk, class InitAccumulatorFn, class ElementParser, class SeparatorParser, class HandleElementFn, class Enable = std::enable_if_t<is_parser_v<ElementParser> && is_parser_v<SeparatorParser>>>
        constexpr auto repsep_(ElementParser&& elementParser, SeparatorParser&& separatorParser, InitAccumulatorFn&& initAccumulatorFn, HandleElementFn&& handleElementFn) {
            return [elementParser = std::forward<ElementParser>(elementParser),
                    separatorParser = std::forward<SeparatorParser>(separatorParser),
                    initAccumulatorFn = std::forward<InitAccumulatorFn>(initAccumulatorFn),
                    handleElementFn = std::forward<HandleElementFn>(handleElementFn)] (Input input) {
                using Accumulator = std::decay_t<decltype(initAccumulatorFn())>;

                auto initResult = elementParser(input);
                if (initResult.is_failure()) {
                    if constexpr (NoMatchIsOk) {
                        // A failure when parsing the first element means we didn't parse any elements.
                        // It's still a success result, just with zero elements.
                        return ParseResult<Accumulator>::success(input);
                    } else {
                        return ParseResult<Accumulator>::failure(input);
                    }
                }
                if (initResult.is_error()) {
                    return ParseResult<Accumulator>::error(initResult.next());
                }
                ASSERT(initResult.is_success());
                Input next = initResult.next();

                ParseResult<Accumulator> result = ParseResult<Accumulator>::success(next, initAccumulatorFn());
                handleElementFn(&result.result(), std::move(initResult).result());

                while(true) {
                    auto separatorResult = separatorParser(result.next());
                    if (separatorResult.is_failure()) {
                        break;
                    }
                    if (separatorResult.is_error()) {
                        result = ParseResult<Accumulator>::error(separatorResult.next());
                        break;
                    }
                    ASSERT(separatorResult.is_success());

                    auto elementResult = elementParser(separatorResult.next());

                    if (elementResult.is_failure()) {
                        break;
                    }
                    if (elementResult.is_error()) {
                        result = ParseResult<Accumulator>::error(elementResult.next());
                        break;
                    }
                    ASSERT(elementResult.is_success());
                    result.setNext(elementResult.next());
                    handleElementFn(&result.result(), std::move(elementResult).result());
                }

                return result;
            };
        }

    }

    constexpr class _compiletime_optimization final {} compiletime_optimization;
    constexpr class _runtime_optimization final {} runtime_optimization;

    template<class InitAccumulatorFn, class ElementParser, class SeparatorParser, class HandleElementFn, class Enable = std::enable_if_t<is_parser_v<ElementParser> && is_parser_v<SeparatorParser>>>
    constexpr auto repsep(ElementParser&& elementParser, SeparatorParser&& separatorParser, InitAccumulatorFn&& initAccumulatorFn, HandleElementFn&& handleElementFn) {
        return details::repsep_<true>(
            std::forward<ElementParser>(elementParser),
            std::forward<SeparatorParser>(separatorParser),
            std::forward<InitAccumulatorFn>(initAccumulatorFn),
            std::forward<HandleElementFn>(handleElementFn)
        );
    }

    template<class Container, class ElementParser, class SeparatorParser, class Enable = std::enable_if_t<is_parser_v<ElementParser> && is_parser_v<SeparatorParser>>>
    constexpr auto repsep(ElementParser&& elementParser, SeparatorParser&& separatorParser, size_t reserveCapacity = 0) {
        return repsep(
            std::forward<ElementParser>(elementParser),
            std::forward<SeparatorParser>(separatorParser),
            [reserveCapacity] () { Container result; result.reserve(reserveCapacity); return result; },
            [] (Container* accumulator, parser_result_t<ElementParser>&& element) {
                accumulator->push_back(static_cast<typename Container::value_type &&>(std::move(element)));
            }
        );
    }

    template<size_t MAX_SIZE = 1024, class ElementParser, class SeparatorParser, class Enable = std::enable_if_t<is_parser_v<ElementParser> && is_parser_v<SeparatorParser>>>
    constexpr auto repsep(_compiletime_optimization, ElementParser&& elementParser, SeparatorParser&& separatorParser) {
        return repsep<details::CompiletimeContainerForParser<ElementParser, MAX_SIZE>, ElementParser, SeparatorParser>(
            std::forward<ElementParser>(elementParser),
            std::forward<SeparatorParser>(separatorParser),
            0 // cvector::reserve() doesn't mean anything
        );
    }

    template<class ElementParser, class SeparatorParser, class Enable = std::enable_if_t<is_parser_v<ElementParser> && is_parser_v<SeparatorParser>>>
    constexpr auto repsep(_runtime_optimization, ElementParser&& elementParser, SeparatorParser&& separatorParser, size_t reserveCapacity = 0) {
        return repsep<details::RuntimeContainerForParser<ElementParser>, ElementParser, SeparatorParser>(
            std::forward<ElementParser>(elementParser),
            std::forward<SeparatorParser>(separatorParser),
            reserveCapacity
        );
    }

    template<class InitAccumulatorFn, class ElementParser, class SeparatorParser, class HandleElementFn, class Enable = std::enable_if_t<is_parser_v<ElementParser> && is_parser_v<SeparatorParser>>>
    constexpr auto repsep1(ElementParser&& elementParser, SeparatorParser&& separatorParser, InitAccumulatorFn&& initAccumulatorFn, HandleElementFn&& handleElementFn) {
        return details::repsep_<false>(
            std::forward<ElementParser>(elementParser),
            std::forward<SeparatorParser>(separatorParser),
            std::forward<InitAccumulatorFn>(initAccumulatorFn),
            std::forward<HandleElementFn>(handleElementFn)
        );
    }

    template<class Container, class ElementParser, class SeparatorParser, class Enable = std::enable_if_t<is_parser_v<ElementParser> && is_parser_v<SeparatorParser>>>
    constexpr auto repsep1(ElementParser&& elementParser, SeparatorParser&& separatorParser, size_t reserveCapacity = 1) {
        return repsep1(
            std::forward<ElementParser>(elementParser),
            std::forward<SeparatorParser>(separatorParser),
            [reserveCapacity] () {Container result; result.reserve(reserveCapacity); return result; },
            [] (Container* accumulator, parser_result_t<ElementParser>&& element) {
                accumulator->push_back(static_cast<typename Container::value_type &&>(std::move(element)));
            }
        );
    }

    template<size_t MAX_SIZE = 1024, class ElementParser, class SeparatorParser, class Enable = std::enable_if_t<is_parser_v<ElementParser> && is_parser_v<SeparatorParser>>>
    constexpr auto repsep1(_compiletime_optimization, ElementParser&& elementParser, SeparatorParser&& separatorParser) {
        return repsep1<details::CompiletimeContainerForParser<ElementParser, MAX_SIZE>, ElementParser, SeparatorParser>(
            std::forward<ElementParser>(elementParser),
            std::forward<SeparatorParser>(separatorParser),
            0 // cvector::reserve() doesn't mean anything
        );
    }

    template<class ElementParser, class SeparatorParser, class Enable = std::enable_if_t<is_parser_v<ElementParser> && is_parser_v<SeparatorParser>>>
    constexpr auto repsep1(_runtime_optimization, ElementParser&& elementParser, SeparatorParser&& separatorParser, size_t reserveCapacity = 1) {
        return repsep1<details::RuntimeContainerForParser<ElementParser>, ElementParser, SeparatorParser>(
            std::forward<ElementParser>(elementParser),
            std::forward<SeparatorParser>(separatorParser),
            reserveCapacity
        );
    }

    template<class InitAccumulatorFn, class ElementParser, class HandleElementFn, class Enable = std::enable_if_t<is_parser_v<ElementParser>>>
    constexpr auto rep(ElementParser&& elementParser, InitAccumulatorFn&& initAccumulatorFn, HandleElementFn&& handleElementFn) {
        return repsep(
            std::forward<ElementParser>(elementParser),
            success(),
            std::forward<InitAccumulatorFn>(initAccumulatorFn),
            std::forward<HandleElementFn>(handleElementFn)
        );
    }

    template<class Container, class Parser, class Enable = std::enable_if_t<is_parser_v<Parser>>>
    constexpr auto rep(Parser&& parser, size_t reserveCapacity = 0) {
        return repsep<Container>(
            std::forward<Parser>(parser),
            success(),
            reserveCapacity
        );
    }

    template<size_t MAX_SIZE = 1024, class Parser, class Enable = std::enable_if_t<is_parser_v<Parser>>>
    constexpr auto rep(_compiletime_optimization, Parser&& parser) {
        return rep<details::CompiletimeContainerForParser<Parser, MAX_SIZE>>(
            std::forward<Parser>(parser),
            0 // cvector::reserve() doesn't mean anything
        );
    }

    template<class Parser, class Enable = std::enable_if_t<is_parser_v<Parser>>>
    constexpr auto rep(_runtime_optimization, Parser&& parser, size_t reserveCapacity = 0) {
        return rep<details::RuntimeContainerForParser<Parser>>(
            std::forward<Parser>(parser),
            reserveCapacity
        );
    }

    template<class InitAccumulatorFn, class ElementParser, class HandleElementFn, class Enable = std::enable_if_t<is_parser_v<ElementParser>>>
    constexpr auto rep1(ElementParser&& elementParser, InitAccumulatorFn&& initAccumulatorFn, HandleElementFn&& handleElementFn) {
        return repsep1(
            std::forward<ElementParser>(elementParser),
            success(),
            std::forward<InitAccumulatorFn>(initAccumulatorFn),
            std::forward<HandleElementFn>(handleElementFn)
        );
    }

    template<class Container, class Parser, class Enable = std::enable_if_t<is_parser_v<Parser>>>
    constexpr auto rep1(Parser&& parser, size_t reserveCapacity = 1) {
        return repsep1<Container>(
            std::forward<Parser>(parser),
            success(),
            reserveCapacity
        );
    }

    template<size_t MAX_SIZE = 1024, class Parser, class Enable = std::enable_if_t<is_parser_v<Parser>>>
    constexpr auto rep1(_compiletime_optimization, Parser&& parser) {
        return rep1<details::CompiletimeContainerForParser<Parser, MAX_SIZE>>(
            std::forward<Parser>(parser),
            0 // cvector::reserve() doesn't mean anything
        );
    }

    template<class Parser, class Enable = std::enable_if_t<is_parser_v<Parser>>>
    constexpr auto rep1(_runtime_optimization, Parser&& parser, size_t reserveCapacity = 1) {
        return rep1<details::RuntimeContainerForParser<Parser>>(
            std::forward<Parser>(parser),
            reserveCapacity
        );
    }

}
