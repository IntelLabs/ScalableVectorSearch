// svs-benchmark
#include "svs-benchmark/benchmark.h"

// svs
#include "svs/core/distance.h"

namespace svsbenchmark::vamana {

SVS_BENCHMARK_FOR_TESTS_ONLY inline float pick_alpha(svs::DistanceType distance) {
    switch (distance) {
        case svs::DistanceType::L2: {
            return 1.2;
        }
        case svs::DistanceType::MIP: {
            return 0.95;
        }
        case svs::DistanceType::Cosine: {
            return 0.95;
        }
    }
    throw ANNEXCEPTION("Unhandled distance type case!");
}

// Test Routines
SVS_BENCHMARK_FOR_TESTS_ONLY inline search::SearchParameters test_search_parameters() {
    return search::SearchParameters{10, {0.2, 0.5, 0.8, 0.9}};
}

SVS_BENCHMARK_FOR_TESTS_ONLY inline std::vector<svs::index::vamana::VamanaSearchParameters>
test_search_configs() {
    return std::vector<svs::index::vamana::VamanaSearchParameters>(
        {{{{10, 20}, false, 1, 1}, {{15, 15}, false, 1, 1}}}
    );
}

} // namespace svsbenchmark::vamana
