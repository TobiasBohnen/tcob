#include "bench.hpp"

using namespace random;

////////////////////////////////////////////////////////////

void static RANDOM_xoroshiro_128_plus(benchmark::State& state)
{
    rng_xoroshiro_128_plus rng {};
    for (auto _ : state) {
        auto x = rng(1, 1000);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(RANDOM_xoroshiro_128_plus)->Iterations(10000000);

////////////////////////////////////////////////////////////

void static RANDOM_xoroshiro_128_plus_plus(benchmark::State& state)
{
    rng_xoroshiro_128_plus_plus rng {};
    for (auto _ : state) {
        auto x = rng(1, 1000);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(RANDOM_xoroshiro_128_plus_plus)->Iterations(10000000);

////////////////////////////////////////////////////////////

void static RANDOM_xoroshiro_128_star_star(benchmark::State& state)
{
    rng_xoroshiro_128_star_star rng {};
    for (auto _ : state) {
        auto x = rng(1, 1000);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(RANDOM_xoroshiro_128_star_star)->Iterations(10000000);

////////////////////////////////////////////////////////////

void static RANDOM_split_mix_64(benchmark::State& state)
{
    rng_split_mix_64 rng {};
    for (auto _ : state) {
        auto x = rng(1, 1000);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(RANDOM_split_mix_64)->Iterations(10000000);

////////////////////////////////////////////////////////////

void static RANDOM_xorshift_64(benchmark::State& state)
{
    rng_xorshift_64 rng {};
    for (auto _ : state) {
        auto x = rng(1, 1000);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(RANDOM_xorshift_64)->Iterations(10000000);

////////////////////////////////////////////////////////////

void static RANDOM_xorshift_64_star(benchmark::State& state)
{
    rng_xorshift_64_star rng {};
    for (auto _ : state) {
        auto x = rng(1, 1000);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(RANDOM_xorshift_64_star)->Iterations(10000000);

////////////////////////////////////////////////////////////

void static RANDOM_xoshiro_256_plus(benchmark::State& state)
{
    rng_xoshiro_256_plus rng {};
    for (auto _ : state) {
        auto x = rng(1, 1000);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(RANDOM_xoshiro_256_plus)->Iterations(10000000);

////////////////////////////////////////////////////////////

void static RANDOM_xoshiro_256_plus_plus(benchmark::State& state)
{
    rng_xoshiro_256_plus_plus rng {};
    for (auto _ : state) {
        auto x = rng(1, 1000);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(RANDOM_xoshiro_256_plus_plus)->Iterations(10000000);

////////////////////////////////////////////////////////////

void static RANDOM_xoshiro_256_star_star(benchmark::State& state)
{
    rng_xoshiro_256_star_star rng {};
    for (auto _ : state) {
        auto x = rng(1, 1000);
        benchmark::DoNotOptimize(x);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(RANDOM_xoshiro_256_star_star)->Iterations(10000000);