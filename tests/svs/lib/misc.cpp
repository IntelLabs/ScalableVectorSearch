/**
 *    Copyright (C) 2023-present, Intel Corporation
 *
 *    You can redistribute and/or modify this software under the terms of the
 *    GNU Affero General Public License version 3.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    version 3 along with this software. If not, see
 *    <https://www.gnu.org/licenses/agpl-3.0.en.html>.
 */

// svs
#include "svs/lib/misc.h"

// catch2
#include "catch2/catch_test_macros.hpp"

// test utils
#include "tests/utils/generators.h"

// stdlib
#include <cmath>
#include <type_traits>
#include <vector>

CATCH_TEST_CASE("Misc", "[core][misc]") {
    CATCH_SECTION("As Span") {
        auto x = std::vector<float>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        auto check = [&x](const auto& y) {
            if (x.size() != y.size()) {
                return false;
            }
            return std::equal(x.begin(), x.end(), y.begin());
        };

        // Dynamic size
        {
            auto y = svs::lib::as_span(x);
            CATCH_REQUIRE(!std::is_const_v<typename decltype(y)::element_type>);
            CATCH_REQUIRE(y.extent == svs::Dynamic);
            check(y);

            y = svs::lib::as_span<svs::Dynamic>(x);
            CATCH_REQUIRE(!std::is_const_v<typename decltype(y)::element_type>);
            CATCH_REQUIRE(y.extent == svs::Dynamic);
            check(y);
        }
        {
            auto y = svs::lib::as_const_span(x);
            CATCH_REQUIRE(std::is_const_v<typename decltype(y)::element_type>);
            CATCH_REQUIRE(y.extent == svs::Dynamic);
            check(y);

            y = svs::lib::as_const_span<svs::Dynamic>(x);
            CATCH_REQUIRE(std::is_const_v<typename decltype(y)::element_type>);
            CATCH_REQUIRE(y.extent == svs::Dynamic);
            check(y);
        }
        {
            auto y = svs::lib::as_span(std::as_const(x));
            CATCH_REQUIRE(std::is_const_v<typename decltype(y)::element_type>);
            CATCH_REQUIRE(y.extent == svs::Dynamic);
            check(y);

            y = svs::lib::as_span<svs::Dynamic>(std::as_const(x));
            CATCH_REQUIRE(std::is_const_v<typename decltype(y)::element_type>);
            CATCH_REQUIRE(y.extent == svs::Dynamic);
            check(y);
        }

        // Static Size
        {
            auto y = svs::lib::as_span<10>(x);
            CATCH_REQUIRE(!std::is_const_v<typename decltype(y)::element_type>);
            CATCH_REQUIRE(y.extent == 10);
            check(y);

            CATCH_REQUIRE_THROWS_AS(svs::lib::as_span<11>(x), svs::ANNException);
        }
        {
            auto y = svs::lib::as_const_span<10>(x);
            CATCH_REQUIRE(std::is_const_v<typename decltype(y)::element_type>);
            CATCH_REQUIRE(y.extent == 10);
            check(y);

            CATCH_REQUIRE_THROWS_AS(svs::lib::as_const_span<11>(x), svs::ANNException);
        }
        {
            auto y = svs::lib::as_span<10>(std::as_const(x));
            CATCH_REQUIRE(std::is_const_v<typename decltype(y)::element_type>);
            CATCH_REQUIRE(y.extent == 10);
            check(y);

            CATCH_REQUIRE_THROWS_AS(
                svs::lib::as_span<11>(std::as_const(x)), svs::ANNException
            );
        }
    }

    CATCH_SECTION("Identity") {
        svs::lib::identity f{};
        CATCH_REQUIRE(f(10) == 10);
        CATCH_REQUIRE(f(uint8_t{5}) == 5);
        CATCH_REQUIRE(std::is_same_v<decltype(f(uint8_t{5})), uint8_t>);

        auto v = std::vector<int>{1, 2, 3, 4, 5};
        auto u = f(v);
        CATCH_REQUIRE(v.size() == u.size());
        CATCH_REQUIRE(std::equal(v.begin(), v.end(), u.begin()));
        // In this case, `f` is going to make a copy of the vector `v`.
        CATCH_REQUIRE(v.data() != u.data());
    }

    CATCH_SECTION("Returns") {
        CATCH_SECTION("Non-Const") {
            svs::lib::Returns x1{10};
            CATCH_REQUIRE(x1(100) == 10);
            CATCH_REQUIRE(x1(100, "hello") == 10);
            CATCH_REQUIRE(x1() == 10);

            // Copy constructor and assignment operators.
            auto x2 = x1;
            CATCH_REQUIRE(x2() == 10);
            x2 = svs::lib::Returns(100);
            CATCH_REQUIRE(x2() == 100);
            CATCH_REQUIRE(x2("hi") == 100);
        }

        CATCH_SECTION("Const") {
            svs::lib::Returns x1{svs::lib::Const<true>()};
            CATCH_REQUIRE(x1());
            CATCH_REQUIRE(x1(5));
            CATCH_REQUIRE(x1(5, 10));
            CATCH_REQUIRE(x1(5, 10, 500));
        }
    }

    CATCH_SECTION("Power of Two") {
        const size_t max_pow_two = 63;
        std::vector<size_t> test_values(1000);
        CATCH_REQUIRE(test_values.size() == 1000);

        for (size_t i = 0; i < max_pow_two; ++i) {
            svs::lib::PowerOfTwo p{i};
            CATCH_REQUIRE(p.raw() == i);
            CATCH_REQUIRE(p.value() == std::pow(2, i));

            // Test "prevpow2"
            size_t value = p.value();
            CATCH_REQUIRE(svs::lib::prevpow2(value).raw() == i);
            if (i > 0) {
                CATCH_REQUIRE(svs::lib::prevpow2(value + 1).raw() == i);
                CATCH_REQUIRE(svs::lib::prevpow2(value - 1).raw() == i - 1);
            }

            // Test division and modulus.
            auto generator = svs_test::make_generator<size_t>(
                value, std::max(value + size_t{1000}, svs::lib::PowerOfTwo{i + 1}.value())
            );
            svs_test::populate(test_values, generator);
            for (auto& v : test_values) {
                CATCH_REQUIRE(v / p == v / value);
                CATCH_REQUIRE(v % p == v % value);
            }

            for (auto& i : {2, 3}) {
                CATCH_REQUIRE(p * i == value * i);
                CATCH_REQUIRE(i * p == i * value);
            }
        }
    }

    CATCH_SECTION("Intersect") {
        // lvalue version
        std::vector<size_t> a{1, 2, 3, 4, 5};
        std::vector<size_t> b{2, 4, 6, 8};
        CATCH_REQUIRE(svs::lib::count_intersect(a, b) == 2);

        // rvalue version
        bool passed = svs::lib::count_intersect(
            std::vector<int64_t>{2, 4, 6}, std::vector<int64_t>{6, 10, 4}
        );
        CATCH_REQUIRE(passed == true);

        // test multiplicity handling
        a = {1, 1, 1, 1, 1, 2, 2, 4};
        b = {1, 1, 2, 2, 2, 2, 3, 3, 4, 4};
        CATCH_REQUIRE(svs::lib::count_intersect(a, b) == 3);
        CATCH_REQUIRE(svs::lib::count_intersect(b, a) == 3);
    }
}
