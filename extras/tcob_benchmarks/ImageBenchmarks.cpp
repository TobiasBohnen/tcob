#include "bench.hpp"

using namespace random;

////////////////////////////////////////////////////////////

void static IMAGE_decode_png(benchmark::State& state)
{
    for (auto _ : state) {
        auto x = image::Load("/testfiles/test1.png");
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(IMAGE_decode_png)->Iterations(100000);