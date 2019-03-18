#include "funcsig_parser/parameter.h"

#include <gtest/gtest.h>

using namespace ctpc::funcsig_parser;

namespace {

    namespace parameter_success_nospace {
        constexpr auto parsed = ctpc::funcsig_parser::parameter()(ctpc::Input{"name:type someotherstuff"});
        static_assert(parsed.is_success());
        static_assert(parsed.result().name == "name");
        static_assert(parsed.result().type == "type");
        static_assert(parsed.next().input == " someotherstuff");
    }

    namespace parameter_success_spaces {
        constexpr auto parsed = ctpc::funcsig_parser::parameter()(ctpc::Input{"name  :  type someotherstuff"});
        static_assert(parsed.is_success());
        static_assert(parsed.result().name == "name");
        static_assert(parsed.result().type == "type");
        static_assert(parsed.next().input == " someotherstuff");
    }

    namespace parameter_failure_namenotidentifier {
        constexpr auto parsed = ctpc::funcsig_parser::parameter()(ctpc::Input{"23:type"});
        static_assert(parsed.is_failure());
        static_assert(parsed.next().input == "23:type");
    }

    namespace parameter_failure_typenotidentifier {
        constexpr auto parsed = ctpc::funcsig_parser::parameter()(ctpc::Input{"name:34"});
        static_assert(parsed.is_failure());
        static_assert(parsed.next().input == "34");
    }

    namespace parameter_failure_noseparator {
        constexpr auto parsed = ctpc::funcsig_parser::parameter()(ctpc::Input{"nametype"});
        static_assert(parsed.is_failure());
        static_assert(parsed.next().input == "");
    }

    namespace parameter_failure_spaceinsteadofseparator {
        constexpr auto parsed = ctpc::funcsig_parser::parameter()(ctpc::Input{"name type"});
        static_assert(parsed.is_failure());
        static_assert(parsed.next().input == "type");
    }

}
