#include <iostream>
#include <functional>
#include <utility>
#include <stdfloat>
#include <cstdint>
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrack/backtrack.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/graph/utils.h"
#include "offbynull/aligner/graph/grid_graph.h"

using offbynull::aligner::concepts::weight;
using offbynull::aligner::backtrack::backtrack::backtracker;
using offbynull::aligner::backtrack::slot_container::slot;
using offbynull::aligner::graph::graph::readable_graph;
using offbynull::aligner::graph::grid_container_creator::grid_container_creator;
using offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator;
using offbynull::aligner::graph::grid_container_creators::array_grid_container_creator;
using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
using offbynull::aligner::backtrack::container_creator::container_creator;
using offbynull::aligner::backtrack::container_creators::vector_container_creator;
using offbynull::aligner::backtrack::container_creators::array_container_creator;
using offbynull::aligner::backtrack::container_creators::static_vector_container_creator;

using ND = std::tuple<>;

template<
    bool error_check=false,
    std::unsigned_integral INDEXER=std::size_t,
    weight ED=std::float64_t
>
auto global(
    std::ranges::range auto&& v,
    std::ranges::range auto&& w,
    std::function<
        ED(
            const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
            const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
        )
    > weight_lookup
) {
    static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
    static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

    using ND_ALLOCATOR=vector_grid_container_creator<ND, INDEXER, error_check>;
    using ED_ALLOCATOR=vector_grid_container_creator<ED, INDEXER, error_check>;
    INDEXER v_node_cnt { v.size() + 1u };
    INDEXER w_node_cnt { w.size() + 1u };
    pairwise_global_alignment_graph<ND, ED, INDEXER, ND_ALLOCATOR, ED_ALLOCATOR, false> graph { v_node_cnt, w_node_cnt };
    graph.template assign_weights<ED>(
        v,
        w,
        weight_lookup,
        [](ED& edge_data, ED weight) { edge_data = weight; }
    );
    using G = decltype(graph);
    using N = typename G::N;
    using E = typename G::E;
    using SLOT_ALLOCATOR=vector_container_creator<slot<N, E, INDEXER, ED>, error_check>;
    using PATH_ALLOCATOR=vector_container_creator<E, error_check>;
    backtracker<G, INDEXER, ED, SLOT_ALLOCATOR, PATH_ALLOCATOR> backtracker_ {};
    using E = typename G::E;
    auto [path, weight] {
        backtracker_.find_max_path(
            graph,
            [&](const E& e) { return graph.get_edge_data(e); }
        )
    };
    return std::make_pair(
        std::move(path)
            | std::views::transform([&](const auto& edge) { return graph.edge_to_elements(edge, v, w); })
            | std::views::filter([](const auto& elem_pair) { return elem_pair.has_value(); })
            | std::views::transform([](const auto& elem_pair) { return *elem_pair; }),
        weight
    );
}

template<
    typename V_ELEM,
    std::size_t V_SIZE,
    typename W_ELEM,
    std::size_t W_SIZE,
    bool error_check=false,
    std::unsigned_integral INDEXER=std::size_t,
    weight ED=std::float64_t
>
auto global_stack(
    std::array<V_ELEM, V_SIZE>& v,
    std::array<W_ELEM, W_SIZE>& w,
    std::function<
        ED(
            const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
            const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
        )
    > weight_lookup
) {
    static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
    static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

    constexpr INDEXER v_node_cnt { V_SIZE + 1u };
    constexpr INDEXER w_node_cnt { W_SIZE + 1u };
    using ND_ALLOCATOR=array_grid_container_creator<ND, INDEXER, v_node_cnt, w_node_cnt, error_check>;
    using ED_ALLOCATOR=array_grid_container_creator<ED, INDEXER, v_node_cnt, w_node_cnt, error_check>;
    pairwise_global_alignment_graph<ND, ED, INDEXER, ND_ALLOCATOR, ED_ALLOCATOR, false> graph { v_node_cnt, w_node_cnt };
    static_assert(offbynull::aligner::graph::grid_graph::readable_grid_graph<decltype(graph), char, char>);
    graph.template assign_weights<ED>(
        v,
        w,
        weight_lookup,
        [](ED& edge_data, ED weight) { edge_data = weight; }
    );
    using G = decltype(graph);
    using N = typename G::N;
    using E = typename G::E;
    using SLOT_ALLOCATOR=array_container_creator<slot<N, E, INDEXER, ED>, G::node_count(v_node_cnt, w_node_cnt), error_check>;
    using PATH_ALLOCATOR=static_vector_container_creator<E, G::longest_path_edge_count(v_node_cnt, w_node_cnt), error_check>;
    backtracker<G, INDEXER, ED, SLOT_ALLOCATOR, PATH_ALLOCATOR> backtracker_ {};
    using E = typename G::E;
    auto [path, weight] {
        backtracker_.find_max_path(
            graph,
            [&](const E& e) { return graph.get_edge_data(e); }
        )
    };
    return std::make_pair(
        std::move(path)
            | std::views::transform([&](const auto& edge) { return graph.edge_to_elements(edge, v, w); })
            | std::views::filter([](const auto& elem_pair) { return elem_pair.has_value(); })
            | std::views::transform([](const auto& elem_pair) { return *elem_pair; }),
        weight
    );
}

int main() {
    auto weight_lookup {
        [](const auto& v_elem, const auto& w_elem) -> std::float16_t {
            if (!v_elem.has_value() || !w_elem.has_value()) {
                return 0.0;
            }
            return v_elem.value().get() == w_elem.value().get() ? 1.0 : 0.0;
        }
    };
    auto print {
        [](auto& elements) {
            std::string v_align {};
            std::string w_align {};
            for (const auto& [v_elem_opt, w_elem_opt] : elements) {
                v_align += (v_elem_opt.has_value() ? *v_elem_opt : '-');
                w_align += (w_elem_opt.has_value() ? *w_elem_opt : '-');
            }
            std::cout << v_align << std::endl;
            std::cout << w_align << std::endl;
        }
    };

    // dynamic
    // std::string v { "hello" };
    // std::string w { "mellow" };
    // auto [elements, weight] {
    //     global<false, std::size_t, std::float64_t>(
    //         v,
    //         w,
    //         weight_lookup
    //     )
    // };
    // static
    std::array<char, 5> v { 'h', 'e', 'l', 'l', 'o' };
    std::array<char, 6> w { 'm', 'e', 'l', 'l', 'o', 'w' };
    volatile auto output {
        global_stack<decltype(v)::value_type, v.size(), decltype(w)::value_type, w.size(), false, std::uint8_t, std::float16_t>(
            v,
            w,
            weight_lookup
        )
    };

    // auto [elements, weight] { output };
    // print(elements);
    return 0;
}