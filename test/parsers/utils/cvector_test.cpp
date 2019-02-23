#include "parsers/utils/cvector.h"
#include <gtest/gtest.h>

using namespace ctpc;

namespace {

template<class Head, class... Tail> constexpr cvector<Head, 1024> make_cvector(Head&& head, Tail&&... tail) {
  cvector<Head, 1024> result;
  result.push_back(std::forward<Head>(head));
  (result.push_back(std::forward<Tail>(tail)), ...);
  return result;
}

template<class Type> constexpr cvector<Type, 1024> make_cvector_maxelem(Type t) {
  cvector<Type, 1024> result;
  for (size_t i = 0; i < 1024; ++i) {
      result.push_back(t);
  }
  return result;
}

constexpr cvector<int, 1024> make_cvector_maxelem_increasing_values() {
  cvector<int, 1024> result;
  for (size_t i = 0; i < 1024; ++i) {
    result.push_back(i);
  }
  return result;
}

template<class Condition, size_t... indices>
constexpr bool true_for_each_index(Condition&& condition, std::index_sequence<indices...>) {
    return (condition(indices) && ...);
}

namespace value_type_test {
  static_assert(std::is_same_v<int, cvector<int, 1024>::value_type>);
  static_assert(std::is_same_v<double, cvector<double, 1024>::value_type>);
}

namespace size_test {
  constexpr cvector<int, 1024> empty;
  static_assert(0 == empty.size());

  constexpr cvector<int, 1024> one_elem = make_cvector(3);
  static_assert(1 == one_elem.size());

  constexpr cvector<int, 1024> two_elem = make_cvector(3, 4);
  static_assert(2 == two_elem.size());

  constexpr cvector<int, 1024> max_elem = make_cvector_maxelem(3);
  static_assert(1024 == max_elem.size());
}

namespace get_test {
  TEST(CVectorTest_GetTest, empty_constexpr) {
    constexpr cvector<int, 1024> empty;
    EXPECT_ANY_THROW(empty[0]);
  }
  TEST(CVectorTest_GetTest, empty_mutable) {
    cvector<int, 1024> empty;
    EXPECT_ANY_THROW(empty[0]);
  }

  TEST(CVectorTest_GetTest, oneElem_constexpr) {
    constexpr cvector<int, 1024> one_elem = make_cvector(3);
    static_assert(3 == one_elem[0]);
    EXPECT_EQ(3, one_elem[0]);
    EXPECT_ANY_THROW(one_elem[1]);
  }
  TEST(CVectorTest_GetTest, oneElem_mutable) {
    cvector<int, 1024> one_elem = make_cvector(3);
    EXPECT_EQ(3, one_elem[0]);
    EXPECT_ANY_THROW(one_elem[1]);

    one_elem[0] = 5;
    EXPECT_EQ(5, one_elem[0]);
    EXPECT_ANY_THROW(one_elem[1]);
  }

  TEST(CVectorTest_GetTest, twoElem_constexpr) {
    constexpr cvector<int, 1024> two_elem = make_cvector(4, 3);
    static_assert(4 == two_elem[0]);
    static_assert(3 == two_elem[1]);
    EXPECT_EQ(4, two_elem[0]);
    EXPECT_EQ(3, two_elem[1]);
    EXPECT_ANY_THROW(two_elem[2]);
  }
  TEST(CVectorTest_GetTest, twoElem_mutable) {
    cvector<int, 1024> two_elem = make_cvector(4, 3);
    EXPECT_EQ(4, two_elem[0]);
    EXPECT_EQ(3, two_elem[1]);
    EXPECT_ANY_THROW(two_elem[2]);

    two_elem[0] = 6;
    two_elem[1] = 7;
    EXPECT_EQ(6, two_elem[0]);
    EXPECT_EQ(7, two_elem[1]);
    EXPECT_ANY_THROW(two_elem[2]);
  }

  TEST(CVectorTest_GetTest, maxElem_constexpr) {
    static constexpr cvector<int, 1024> max_elem = make_cvector_maxelem(4);
    static_assert(true_for_each_index([] (size_t index) {return 4 == max_elem[index];}, std::make_index_sequence<1024>()));
    for (size_t i = 0; i < 1024; ++i) {
      EXPECT_EQ(4, max_elem[i]);
    }
  }
  TEST(CVectorTest_GetTest, maxElem_mutable) {
    cvector<int, 1024> max_elem = make_cvector_maxelem(4);
    for (size_t i = 0; i < 1024; ++i) {
      EXPECT_EQ(4, max_elem[i]);
    }

    // and mutate
    for (size_t i = 0; i < 1024; ++i) {
      max_elem[i] = 1024 - i;
    };
    for (size_t i = 0; i < 1024; ++i) {
      EXPECT_EQ(1024-i, max_elem[i]);
    }
  }
}

namespace get_unsafe_test {
    constexpr cvector<int, 1024> one_elem = make_cvector(3);
    static_assert(3 == one_elem.get_unsafe(0));

    constexpr cvector<int, 1024> two_elem = make_cvector(4, 3);
    static_assert(4 == two_elem.get_unsafe(0));
    static_assert(3 == two_elem.get_unsafe(1));

    constexpr cvector<int, 1024> max_elem = make_cvector_maxelem(4);
    static_assert(true_for_each_index([] (size_t index) {return 4 == max_elem.get_unsafe(index);}, std::make_index_sequence<1024>()));
}

namespace iterator_test {
  constexpr cvector<int, 1024> empty;
  static_assert(empty.begin() == empty.end());
  TEST(CVectorTest_IteratorTest, givenEmpty_testMutableBeginEnd) {
    cvector<int, 1024> empty;
    EXPECT_EQ(empty.begin(), empty.end());
  }

  constexpr cvector<int, 1024> one_elem = make_cvector(3);
  static_assert(1 == one_elem.end() - one_elem.begin());
  static_assert(3 == *one_elem.begin());
  TEST(CVectorTest_IteratorTest, givenOneElem_testMutableBeginEnd) {
    cvector<int, 1024> one_elem = make_cvector(3);
    EXPECT_EQ(1, one_elem.end() - one_elem.begin());
    EXPECT_EQ(3, *one_elem.begin());

    *one_elem.begin() = 5;
    EXPECT_EQ(5, one_elem[0]);
  }

  constexpr cvector<int, 1024> two_elem = make_cvector(4, 3);
  static_assert(2 == two_elem.end() - two_elem.begin());
  static_assert(4 == *two_elem.begin());
  static_assert(3 == *(two_elem.begin()+1));
  TEST(CVectorTest_IteratorTest, givenTwoElem_testMutableBeginEnd) {
    cvector<int, 1024> two_elem = make_cvector(4, 3);
    EXPECT_EQ(2, two_elem.end() - two_elem.begin());
    EXPECT_EQ(4, *two_elem.begin());
    EXPECT_EQ(3, *(two_elem.begin()+1));

    *two_elem.begin() = 5;
    *(two_elem.begin()+1) = 6;
    EXPECT_EQ(5, two_elem[0]);
    EXPECT_EQ(6, two_elem[1]);
  }

  constexpr cvector<int, 1024> max_elem = make_cvector_maxelem(4);
  static_assert(1024 == max_elem.end() - max_elem.begin());
  static_assert(true_for_each_index([] (size_t index) {return 4 == *(max_elem.begin() + index);}, std::make_index_sequence<1024>()));
  TEST(CVectorTest_IteratorTest, givenMaxElem_testMutableBeginEnd) {
    cvector<int, 1024> max_elem = make_cvector_maxelem(5);
    EXPECT_EQ(1024, max_elem.end() - max_elem.begin());
    for (int& elem : max_elem) {
      EXPECT_EQ(5, elem);
    }

    for (int& elem : max_elem) {
      elem = 7;
    }
    for (int& elem : max_elem) {
      EXPECT_EQ(7, elem);
    }

    for (size_t i = 0; i < 1024; ++i) {
      *(max_elem.begin()+i) = i;
    }
    for (size_t i = 0; i < 1024; ++i) {
      EXPECT_EQ(i, *(max_elem.begin()+i));
    }
  }
}

namespace push_back_test {
  TEST(CVectorTest_PushBack, testPushBack) {
    cvector<int, 1024> obj;
    EXPECT_EQ(0, obj.size());
    for (size_t i = 0; i < 1024; ++i) {
      obj.push_back(1024-i);
      EXPECT_EQ(i+1, obj.size());
      // test it didn't change previously existing elements
      for (size_t j = 0; j <= i; ++j) {
        EXPECT_EQ(1024-j, obj[j]);
      }
    }

    // try to grow over capacity
    EXPECT_ANY_THROW(obj.push_back(0));
  }
}

namespace to_vector_test {
  constexpr cvector<int, 1024> empty;
  TEST(CVectorTest_ToVector, empty) {
    std::vector<int> converted = cvector_to_vector<empty.size()>(empty);
    EXPECT_EQ(0, converted.size());
  }

  constexpr cvector<int, 1024> one_elem = make_cvector(3);
  TEST(CVectorTest_ToVector, oneElem) {
      std::vector<int> converted = cvector_to_vector<one_elem.size()>(one_elem);
      EXPECT_EQ(1, converted.size());
      EXPECT_EQ(3, converted[0]);
  }

  constexpr cvector<int, 1024> two_elem = make_cvector(4, 3);
  TEST(CVectorTest_ToVector, twoElem) {
    std::vector<int> converted = cvector_to_vector<two_elem.size()>(two_elem);
    EXPECT_EQ(2, converted.size());
    EXPECT_EQ(4, converted[0]);
    EXPECT_EQ(3, converted[1]);
  }

  constexpr cvector<int, 1024> max_elem_one_value = make_cvector_maxelem(5);
  TEST(CVectorTest_ToVector, maxElem_oneValue) {
    std::vector<int> converted = cvector_to_vector<max_elem_one_value.size()>(max_elem_one_value);
    EXPECT_EQ(1024, converted.size());
    for (size_t i = 0; i < 1024; ++i){
      EXPECT_EQ(5, converted[i]);
    }
  }

  constexpr cvector<int, 1024> max_elem_increasing_values = make_cvector_maxelem_increasing_values();
  TEST(CVectorTest_ToVector, maxElem_increasingValues) {
    std::vector<int> converted = cvector_to_vector<max_elem_increasing_values.size()>(max_elem_increasing_values);
    EXPECT_EQ(1024, converted.size());
    for (size_t i = 0; i < 1024; ++i){
      EXPECT_EQ(i, converted[i]);
    }
  }
}

namespace aggregate_initialization_test {
    constexpr cvector<int, 1024> empty_0({});
    static_assert(0 == empty_0.size());

    constexpr cvector<int, 1024> empty_1{};
    static_assert(0 == empty_1.size());

    constexpr cvector<int, 1024> one_elem_0({3});
    static_assert(1 == one_elem_0.size());
    static_assert(3 == one_elem_0[0]);

    constexpr cvector<int, 1024> one_elem_1{3};
    static_assert(1 == one_elem_1.size());
    static_assert(3 == one_elem_1[0]);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbraced-scalar-init"
    constexpr cvector<int, 1024> one_elem_2{{3}};
#pragma GCC diagnostic pop
    static_assert(1 == one_elem_2.size());
    static_assert(3 == one_elem_2[0]);

    constexpr cvector<int, 1024> two_elem_0({4, 3});
    static_assert(2 == two_elem_0.size());
    static_assert(4 == two_elem_0[0]);
    static_assert(3 == two_elem_0[1]);

    constexpr cvector<int, 1024> two_elem_1{4, 3};
    static_assert(2 == two_elem_1.size());
    static_assert(4 == two_elem_1[0]);
    static_assert(3 == two_elem_1[1]);

    constexpr cvector<int, 1024> two_elem_2{{4, 3}};
    static_assert(2 == two_elem_2.size());
    static_assert(4 == two_elem_2[0]);
    static_assert(3 == two_elem_2[1]);
}

}
