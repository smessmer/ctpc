#pragma once

#include <stdexcept>

namespace ctpc {

constexpr void ASSERT(bool condition) {
    if (!condition) {
        throw std::runtime_error("Assertion failed");
    }
}

}
