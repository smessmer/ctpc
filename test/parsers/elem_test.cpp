#include "parsers/elem.h"

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


}
