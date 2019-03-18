#include "funcsig_parser/function_signature.h"
#include "parsers/phrase.h"

#include <gtest/gtest.h>

using namespace ctpc;
using namespace ctpc::funcsig_parser;

namespace {

namespace function_signature_success_no_params {
    constexpr auto parsed = phrase(function_signature(compiletime_optimization))(Input{"Int some_other_func()"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().return_type == "Int");
    static_assert(parsed.result().name == "some_other_func");
    static_assert(parsed.result().parameters.size() == 0);
}
namespace function_signature_success_no_params_whitespaces {
    constexpr auto parsed = phrase(function_signature(compiletime_optimization))(Input{"Int  some_other_func  (  )"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().return_type == "Int");
    static_assert(parsed.result().name == "some_other_func");
    static_assert(parsed.result().parameters.size() == 0);
}
namespace function_signature_success_one_param {
    constexpr auto parsed = phrase(function_signature(compiletime_optimization))(Input{"void my_func(arg1: Int)"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().return_type == "void");
    static_assert(parsed.result().name == "my_func");
    static_assert(parsed.result().parameters.size() == 1);
    static_assert(parsed.result().parameters[0].name == "arg1");
    static_assert(parsed.result().parameters[0].type == "Int");
}
namespace function_signature_success_two_params {
    constexpr auto parsed = phrase(function_signature(compiletime_optimization))(Input{"void my_func(arg1: Int, arg2: String)"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().return_type == "void");
    static_assert(parsed.result().name == "my_func");
    static_assert(parsed.result().parameters.size() == 2);
    static_assert(parsed.result().parameters[0].name == "arg1");
    static_assert(parsed.result().parameters[0].type == "Int");
    static_assert(parsed.result().parameters[1].name == "arg2");
    static_assert(parsed.result().parameters[1].type == "String");
}
namespace function_signature_success_whitespaces {
    constexpr auto parsed = phrase(function_signature(compiletime_optimization))(Input{"Int  my_other_func  (  arg1  :  String  ,  arg2  :  Double  )"});
    static_assert(parsed.is_success());
    static_assert(parsed.result().return_type == "Int");
    static_assert(parsed.result().name == "my_other_func");
    static_assert(parsed.result().parameters.size() == 2);
    static_assert(parsed.result().parameters[0].name == "arg1");
    static_assert(parsed.result().parameters[0].type == "String");
    static_assert(parsed.result().parameters[1].name == "arg2");
    static_assert(parsed.result().parameters[1].type == "Double");
}

namespace function_signature_failure_returntype {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"-invalid- my_func(arg1: Int, arg2: String)"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "-invalid- my_func(arg1: Int, arg2: String)");
}
namespace function_signature_failure_noopeningparens {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"void my_funcarg1: Int, arg2: String)"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == ": Int, arg2: String)");
}
namespace function_signature_failure_arg1invalidname {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"void my_func(-invalid-: Int, arg2: String)"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "-invalid-: Int, arg2: String)");
}
namespace function_signature_failure_arg1nocolon {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"void my_func(arg1 Int, arg2: String)"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "arg1 Int, arg2: String)");
}
namespace function_signature_failure_arg1invalidtype {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"void my_func(arg1: -invalid-, arg2: String)"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "arg1: -invalid-, arg2: String)");
}
namespace function_signature_failure_arg1nocomma {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"void my_func(arg1: type arg2: String)"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "arg2: String)");
}
namespace function_signature_failure_arg2invalidname {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"void my_func(arg1: type, -invalid-: String)"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == ", -invalid-: String)");
}
namespace function_signature_failure_arg2nocolon {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"void my_func(arg1: type, arg2 String)"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == ", arg2 String)");
}
namespace function_signature_failure_arg2invalidtype {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"void my_func(arg1: type, arg2: -invalid-)"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == ", arg2: -invalid-)");
}
namespace function_signature_failure_noarg3aftercomma {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"void my_func(arg1: type, arg2: String,)"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == ",)");
}
namespace function_signature_failure_noclosingparen {
    constexpr auto parsed = function_signature(compiletime_optimization)(Input{"void my_func(arg1: type, arg2: String"});
    static_assert(parsed.is_failure());
    static_assert(parsed.next().input == "");
}

}
