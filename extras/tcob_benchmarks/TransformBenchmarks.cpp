#include "bench.hpp"

////////////////////////////////////////////////////////////

void static TRANSFORM_TransformPoint(benchmark::State& state)
{
    tcob::transform t {};
    t.rotate({45});
    t.scale({0.6f, 5.f});
    t.translate({20.f, 34.f});

    std::vector<point_f> points {
        // clang-format off
        {1.f, 1.1f}, {1.f, 1.2f}, {1.f, 1.3f}, {1.f, 1.4f}, {1.f, 1.5f}, {1.f, 1.6f}, {1.f, 1.7f}, 
        {1.f, 1.8f}, {1.f, 1.9f}, {1.f, 1.10f}, {1.f, 1.11f}, {1.f, 1.12f}, {1.f, 1.13f}, {1.f, 1.14f}, 
        {1.f, 1.15f}, {1.f, 1.16f}, {1.f, 1.17f}, {1.f, 1.18f}, {1.f, 1.19f}, {1.f, 1.20f}, {1.f, 1.21f},
        {1.f, 1.1f}, {1.f, 1.2f}, {1.f, 1.3f}, {1.f, 1.4f}, {1.f, 1.5f}, {1.f, 1.6f}, {1.f, 1.7f}, 
        {1.f, 1.8f}, {1.f, 1.9f}, {1.f, 1.10f}, {1.f, 1.11f}, {1.f, 1.12f}, {1.f, 1.13f}, {1.f, 1.14f}, 
        {1.f, 1.15f}, {1.f, 1.16f}, {1.f, 1.17f}, {1.f, 1.18f}, {1.f, 1.19f}, {1.f, 1.20f}, {1.f, 1.21f},
        {1.f, 1.1f}, {1.f, 1.2f}, {1.f, 1.3f}, {1.f, 1.4f}, {1.f, 1.5f}, {1.f, 1.6f}, {1.f, 1.7f}, 
        {1.f, 1.8f}, {1.f, 1.9f}, {1.f, 1.10f}, {1.f, 1.11f}, {1.f, 1.12f}, {1.f, 1.13f}, {1.f, 1.14f}, 
        {1.f, 1.15f}, {1.f, 1.16f}, {1.f, 1.17f}, {1.f, 1.18f}, {1.f, 1.19f}, {1.f, 1.20f}, {1.f, 1.21f},
        {1.f, 1.1f}, {1.f, 1.2f}, {1.f, 1.3f}, {1.f, 1.4f}, {1.f, 1.5f}, {1.f, 1.6f}, {1.f, 1.7f}, 
        {1.f, 1.8f}, {1.f, 1.9f}, {1.f, 1.10f}, {1.f, 1.11f}, {1.f, 1.12f}, {1.f, 1.13f}, {1.f, 1.14f}, 
        {1.f, 1.15f}, {1.f, 1.16f}, {1.f, 1.17f}, {1.f, 1.18f}, {1.f, 1.19f}, {1.f, 1.20f}, {1.f, 1.21f},
        // clang-format on
    };

    for (auto _ : state) {
        for (auto& point : points) {
            auto x = t * point;
            benchmark::DoNotOptimize(x);
        }
    }
    state.SetItemsProcessed(state.iterations() * points.size());
}
BENCHMARK(TRANSFORM_TransformPoint)->Iterations(1000000);

////////////////////////////////////////////////////////////
