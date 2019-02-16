#pragma once

#include <array>
#include <vector>

namespace ctpc {

    /**
     * cvector is a compile time alternative for std::vector, i.e. a dynamically sized array that can grow and shrink.
     *
     * Since heap allocations at compile time are impossible, cvector is backed by a std::array with a preallocated
     * maximum size and cvector cannot grow past this maximum size.
     *
     * However, cvector is not meant to be instantiated at runtime and the compiler should strip it away in most cases,
     * meaning a large MAX_SIZE doesn't have any runtime impact.
     *
     * However, it's better to verify that in your concrete use case since it's easy to make a mistake and accidentally
     * access the cvector at runtime, in which case suddenly it does exist at runtime.
     */
    template<class T, size_t MAX_SIZE>
    class cvector final {
    public:
        using element_type = T;

        constexpr explicit cvector()
        : elements_(), size_(0) {}

        constexpr explicit cvector(std::initializer_list<T> elements)
        : elements_(), size_(elements.size()) {
            for (size_t i = 0; i < elements.size(); ++i) {
                elements_[i] = *(elements.begin()+i);
            }
        }

        constexpr const T& operator[](size_t index) const {
            if (index >= size_) {
                throw std::runtime_error("out of bounds");
            }
            return elements_[index];
        }

        constexpr T& operator[](size_t index) {
            return const_cast<T&>(const_cast<const cvector*>(this)->operator[](index));
        }

        constexpr const T& get_unsafe(size_t index) const {
            return elements_[index];
        }

        constexpr T& get_unsafe(size_t index) {
            return const_cast<T&>(const_cast<const cvector*>(this)->get_unsafe(index));
        }

        constexpr const T* begin() const {
            return elements_.begin();
        }

        constexpr T* begin() {
            return const_cast<T*>(const_cast<const cvector*>(this)->begin());
        }

        constexpr const T* end() const {
            return elements_.begin() + size_;
        }

        constexpr T* end() {
            return const_cast<T*>(const_cast<const cvector*>(this)->end());
        }

        constexpr size_t size() const {
            return size_;
        }

        constexpr void push_back(T elem) {
            if (size_ == elements_.size()) {
                throw std::runtime_error("No capacity left");
            }

            elements_[size_] = std::move(elem);
            ++size_;
        }

    private:
        std::array<T, MAX_SIZE> elements_;
        size_t size_;
    };

    namespace detail {
        template<class CVEC, size_t... indices>
        inline std::vector<typename CVEC::element_type>
        _cvector_to_vector(const CVEC &vec, std::index_sequence<indices...>) {
            return {vec.get_unsafe(indices)...};
        }
    }

    template<size_t SIZE, class CVEC>
    inline std::vector<typename CVEC::element_type> cvector_to_vector(const CVEC &vec) {
        assert(SIZE == vec.size());
        return detail::_cvector_to_vector(vec, std::make_index_sequence<SIZE>());
    }

}
