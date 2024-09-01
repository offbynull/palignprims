#include <string>
#include <ranges>
#include <cstdint>
#include <chrono>
#include <iostream>
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/scorers/qwerty_scorer.h"
#include "offbynull/aligner/scorers/constant_scorer.h"
#include "offbynull/aligner/sequences/sliding_window_sequence.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker.h"

int main(int /*argc*/, char** /*argv*/) {
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::create_pairwise_global_alignment_graph;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::stack_find_max_path;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::backtracker::heap_find_max_path;
    using offbynull::aligner::scorers::qwerty_scorer::qwerty_scorer;
    using offbynull::aligner::scorers::constant_scorer::constant_scorer;
    using offbynull::aligner::sequences::sliding_window_sequence::create_stack_sliding_window_sequence;
    using offbynull::aligner::sequences::sliding_window_sequence::create_heap_sliding_window_sequence;

#if defined(BENCHMARK_UINT8_INDEX) && defined(BENCHMARK_SIZE_T_INDEX)
    static_assert(false, "Only one must be defined: BENCHMARK_UINT8_INDEX or BENCHMARK_SIZE_T_INDEX");
#elif defined(BENCHMARK_UINT8_INDEX)
    using PARENT_COUNT = std::uint8_t;
    using SLOT_INDEX = std::uint8_t;
#elif defined(BENCHMARK_SIZE_T_INDEX)
    using PARENT_COUNT = std::size_t;
    using SLOT_INDEX = std::size_t;
#else
    static_assert(false, "One must be defined: BENCHMARK_UINT8_INDEX or BENCHMARK_SIZE_T_INDEX");
#endif

    qwerty_scorer<false, float> substitution_scorer {};
    constant_scorer<false, char, char, float> indel_scorer { 0.0f };
    const std::string chars { "abcdefghijklmnopqrstuvwxyz0123456789" };
    const std::string test_string { "zazaza" };
    auto test_string_windows { create_stack_sliding_window_sequence<false, 4zu>(test_string) };
    volatile float unused {}; // variable used to prevent the compiler from optimizing important stuff out
    auto before_tp { std::chrono::steady_clock::now() };
    for (std::size_t i { 0zu }; i < test_string_windows.size(); ++i) {
        auto a { test_string_windows[i] };
        for (auto&& [b1, b2, b3, b4] : std::views::cartesian_product(chars, chars, chars, chars)) {
            std::array<char, 4> b { b1, b2, b3, b4 };
            auto graph {
                create_pairwise_global_alignment_graph<false, std::size_t>(
                    a,
                    b,
                    substitution_scorer,
                    indel_scorer
                )
            };
#if defined(BENCHMARK_HEAP) && defined(BENCHMARK_STACK)
            static_assert(false, "Only one must be defined: BENCHMARK_HEAP or BENCHMARK_STACK");
#elif defined(BENCHMARK_HEAP)
#if defined(MINIMIZE_ALLOCS)
            constexpr bool minimize_allocs { true };
#else
            constexpr bool minimize_allocs { false };
#endif
            auto path { heap_find_max_path<false, PARENT_COUNT, SLOT_INDEX, minimize_allocs>(graph) };
#elif defined(BENCHMARK_STACK)
            auto path { stack_find_max_path<false, PARENT_COUNT, SLOT_INDEX, 4zu, 4zu, 1zu, 8zu>(graph) };
#else
            static_assert(false, "One must be defined: BENCHMARK_HEAP or BENCHMARK_STACK");
#endif
            unused += std::get<1>(path);
        }
    }
    auto after_tp { std::chrono::steady_clock::now() };
    std::chrono::duration duration { after_tp - before_tp };
    std::cout << std::chrono::round<std::chrono::milliseconds>(duration) << std::endl;
    return 0;
}
