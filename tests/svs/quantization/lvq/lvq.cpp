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

// Header under test.
#include "svs/quantization/lvq/lvq.h"

// Extras
#include "svs/lib/saveload.h"

// test utilities
#include "tests/utils/test_dataset.h"
#include "tests/utils/utils.h"

// catch2
#include "catch2/catch_test_macros.hpp"

namespace lvq = svs::quantization::lvq;
using DistanceL2 = svs::distance::DistanceL2;
using DistanceIP = svs::distance::DistanceIP;

namespace {

template <typename T> inline constexpr bool is_distance_main_residual_instance = false;

template <typename Distance, typename Primary, typename Residual>
inline constexpr bool is_distance_main_residual_instance<
    lvq::DistanceMainResidual<Distance, Primary, Residual>> = true;

template <typename T, size_t N, typename Distance> void test_lvq_top() {
    // First, construct an online compression.
    auto vector_data_file = svs::VectorDataLoader<float, N>{test_dataset::data_svs_file()};
    auto loader = T{vector_data_file};

    auto bundle = loader.load(Distance());
    static_assert(is_distance_main_residual_instance<decltype(bundle)>);

    // Try saving and reloading.
    svs_test::prepare_temp_directory();
    auto temp_dir = svs_test::temp_directory();
    svs::lib::save(bundle, temp_dir);
    loader = T{lvq::Reload(temp_dir)};
    auto reloaded_bundle = loader.load(Distance());

    static_assert(std::is_same_v<decltype(bundle), decltype(reloaded_bundle)>);
    CATCH_REQUIRE((bundle.distance == reloaded_bundle.distance));
}

} // namespace

CATCH_TEST_CASE("End-to-End Vector Quantization", "[quantization][lvq]") {
    CATCH_SECTION("OnlineCompression") {
        // Make sure we can construct an instance of "OnlineCompression" using one of the
        // "blessed" source types.
        auto x = lvq::OnlineCompression("a path!", svs::DataType::float32);
        CATCH_REQUIRE(x.path == "a path!");
        CATCH_REQUIRE(x.type == svs::DataType::float32);

        x = lvq::OnlineCompression("another path!", svs::DataType::float16);
        CATCH_REQUIRE(x.path == "another path!");
        CATCH_REQUIRE(x.type == svs::DataType::float16);

        // Incompatible type.
        CATCH_REQUIRE_THROWS_AS(
            lvq::OnlineCompression("another path!", svs::DataType::float64),
            svs::ANNException
        );
    }

    // One Level Compression.
    CATCH_SECTION("One Level Compression") {
        test_lvq_top<lvq::OneLevelWithBias<8, 128>, 128, DistanceL2>();
        test_lvq_top<lvq::OneLevelWithBias<8, 128>, 128, DistanceIP>();
        test_lvq_top<lvq::OneLevelWithBias<4, 128>, 128, DistanceL2>();
        test_lvq_top<lvq::OneLevelWithBias<4, 128>, 128, DistanceIP>();
    }

    CATCH_SECTION("Two Level Compression") {
        test_lvq_top<lvq::TwoLevelWithBias<4, 4, 128>, 128, DistanceL2>();
        test_lvq_top<lvq::TwoLevelWithBias<4, 4, 128>, 128, DistanceIP>();
    }
}
