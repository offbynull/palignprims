#ifndef OFFBYNULL_ALIGNER_ALIGNER_H
#define OFFBYNULL_ALIGNER_ALIGNER_H

#include <functional>
#include <utility>
#include <cstdint>
#include <type_traits>
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrack/backtrack.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_global_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_local_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h"
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::aligner {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrack::backtrack::backtracker;
    using offbynull::aligner::backtrack::slot_container::slot;
    using offbynull::aligner::graph::graph::readable_graph;
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_parwise_alignment_graph;
    using offbynull::aligner::graph::grid_container_creator::grid_container_creator;
    using offbynull::aligner::graph::grid_container_creators::vector_grid_container_creator;
    using offbynull::aligner::graph::grid_container_creators::array_grid_container_creator;
    using offbynull::aligner::graphs::pairwise_global_alignment_graph::pairwise_global_alignment_graph;
    using offbynull::aligner::graphs::pairwise_local_alignment_graph::pairwise_local_alignment_graph;
    using offbynull::aligner::graphs::pairwise_fitting_alignment_graph::pairwise_fitting_alignment_graph;
    using offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::pairwise_extended_gap_alignment_graph;
    template<typename ND, typename ED>
    using extended_gap_slot = offbynull::aligner::graphs::pairwise_extended_gap_alignment_graph::slot<ND, ED>;
    using offbynull::aligner::backtrack::container_creator::container_creator;
    using offbynull::aligner::backtrack::container_creators::vector_container_creator;
    using offbynull::aligner::backtrack::container_creators::array_container_creator;
    using offbynull::aligner::backtrack::container_creators::static_vector_container_creator;
    using offbynull::concepts::widenable_to_size_t;

    template<typename T>
    struct type_encapsulator{
        using type = T;
    };

    template<
        weight WEIGHT,
        bool error_check
    >
    auto align_heap(
        readable_parwise_alignment_graph auto& graph,
        std::ranges::random_access_range auto&& v,
        std::ranges::random_access_range auto&& w
    ) {
        static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
        static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

        using G = std::decay_t<decltype(graph)>;
        using N = typename G::N;
        using E = typename G::E;
        using COUNT = std::size_t;
        using SLOT_ALLOCATOR=vector_container_creator<slot<N, E, COUNT, WEIGHT>, error_check>;
        using PATH_ALLOCATOR=vector_container_creator<E, error_check>;
        backtracker<G, COUNT, WEIGHT, SLOT_ALLOCATOR, PATH_ALLOCATOR> backtracker_ {};
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
        widenable_to_size_t INDEX,  // C++ won't let you infer this because it relies on v/w's size, qhich aren't known at compile-time
        weight WEIGHT,
        bool error_check
    >
    auto global_align_heap(
        std::ranges::random_access_range auto&& v,
        std::ranges::random_access_range auto&& w,
        std::function<
            WEIGHT(
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
            )
        > weight_lookup
    ) {
        static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
        static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

        INDEX v_node_cnt { static_cast<INDEX>(v.size() + 1zu) };
        INDEX w_node_cnt { static_cast<INDEX>(w.size() + 1zu) };
        if constexpr (error_check) {
            if (v_node_cnt != v.size() + 1zu || w_node_cnt != w.size() + 1zu) {
                throw std::runtime_error("Index type too narrow");
            }
        }
        using ND = std::tuple<>;
        using ED = WEIGHT;
        pairwise_global_alignment_graph<
            ND,
            ED,
            INDEX,
            vector_grid_container_creator<ND, INDEX, error_check>,
            vector_grid_container_creator<ED, INDEX, error_check>,
            error_check
        > graph { v_node_cnt, w_node_cnt };
        graph.template assign_weights<WEIGHT>(
            v,
            w,
            weight_lookup,
            [](WEIGHT& edge_data, WEIGHT weight) { edge_data = weight; }
        );
        return align_heap<WEIGHT, error_check>(graph, v, w);
    }

    template<
        widenable_to_size_t INDEX,  // C++ won't let you infer this because it relies on v/w's size, qhich aren't known at compile-time
        weight WEIGHT,
        bool error_check
    >
    auto local_align_heap(
        std::ranges::random_access_range auto&& v,
        std::ranges::random_access_range auto&& w,
        std::function<
            WEIGHT(
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
            )
        > weight_lookup,
        WEIGHT freeride_weight = {}
    ) {
        static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
        static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

        INDEX v_node_cnt { static_cast<INDEX>(v.size() + 1zu) };
        INDEX w_node_cnt { static_cast<INDEX>(w.size() + 1zu) };
        if constexpr (error_check) {
            if (v_node_cnt != v.size() + 1zu || w_node_cnt != w.size() + 1zu) {
                throw std::runtime_error("Index type too narrow");
            }
        }
        using ND = std::tuple<>;
        using ED = WEIGHT;
        pairwise_local_alignment_graph<
            ND,
            ED,
            INDEX,
            vector_grid_container_creator<ND, INDEX, error_check>,
            vector_grid_container_creator<ED, INDEX, error_check>,
            error_check
        > graph { v_node_cnt, w_node_cnt };
        graph.template assign_weights<WEIGHT>(
            v,
            w,
            weight_lookup,
            [](WEIGHT& edge_data, WEIGHT weight) { edge_data = weight; },
            freeride_weight
        );
        return align_heap<WEIGHT, error_check>(graph, v, w);
    }

    template<
        widenable_to_size_t INDEX,  // C++ won't let you infer this because it relies on v/w's size, qhich aren't known at compile-time
        weight WEIGHT,
        bool error_check
    >
    auto fitting_align_heap(
        std::ranges::random_access_range auto&& v,
        std::ranges::random_access_range auto&& w,
        std::function<
            WEIGHT(
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
            )
        > weight_lookup,
        WEIGHT freeride_weight = {}
    ) {
        static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
        static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

        INDEX v_node_cnt { static_cast<INDEX>(v.size() + 1zu) };
        INDEX w_node_cnt { static_cast<INDEX>(w.size() + 1zu) };
        if constexpr (error_check) {
            if (v_node_cnt != v.size() + 1zu || w_node_cnt != w.size() + 1zu) {
                throw std::runtime_error("Index type too narrow");
            }
        }
        using ND = std::tuple<>;
        using ED = WEIGHT;
        pairwise_fitting_alignment_graph<
            ND,
            ED,
            INDEX,
            vector_grid_container_creator<ND, INDEX, error_check>,
            vector_grid_container_creator<ED, INDEX, error_check>,
            error_check
        > graph { v_node_cnt, w_node_cnt };
        graph.template assign_weights<WEIGHT>(
            v,
            w,
            weight_lookup,
            [](WEIGHT& edge_data, WEIGHT weight) { edge_data = weight; },
            freeride_weight
        );
        return align_heap<WEIGHT, error_check>(graph, v, w);
    }


    template<
        widenable_to_size_t INDEX,  // C++ won't let you infer this because it relies on v/w's size, qhich aren't known at compile-time
        weight WEIGHT,
        bool error_check
    >
    auto extended_gap_align_heap(
        std::ranges::random_access_range auto&& v,
        std::ranges::random_access_range auto&& w,
        std::function<
            WEIGHT(
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
            )
        > weight_lookup,
        WEIGHT gap_weight = {},
        WEIGHT freeride_weight = {}
    ) {
        static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
        static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

        INDEX v_slot_cnt { static_cast<INDEX>(v.size() + 1zu) };
        INDEX w_slot_cnt { static_cast<INDEX>(w.size() + 1zu) };
        if constexpr (error_check) {
            if (v_slot_cnt != v.size() + 1zu || w_slot_cnt != w.size() + 1zu) {
                throw std::runtime_error("Index type too narrow");
            }
        }
        using ND = std::tuple<>;
        using ED = WEIGHT;
        pairwise_extended_gap_alignment_graph<
            ND,
            ED,
            INDEX,
            vector_grid_container_creator<
                extended_gap_slot<ND, ED>,
                INDEX,
                error_check
            >,
            error_check
        > graph { v_slot_cnt, w_slot_cnt };
        graph.template assign_weights<WEIGHT>(
            v,
            w,
            weight_lookup,
            [](WEIGHT& edge_data, WEIGHT weight) { edge_data = weight; },
            gap_weight,
            freeride_weight
        );
        return align_heap<WEIGHT, error_check>(graph, v, w);
    }
















    template<
        weight WEIGHT,
        std::size_t V_SIZE,
        std::size_t W_SIZE,
        bool error_check
    >
    auto align_stack(
        readable_parwise_alignment_graph auto& graph,
        std::ranges::random_access_range auto&& v,
        std::ranges::random_access_range auto&& w
    ) {
        static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
        static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

        if constexpr (error_check) {
            if (V_SIZE != v.size() || W_SIZE != w.size()) {
                throw std::runtime_error("Size mismatch");
            }
        }
        constexpr std::size_t v_node_cnt { V_SIZE + 1u };
        constexpr std::size_t w_node_cnt { W_SIZE + 1u };
        using G = std::decay_t<decltype(graph)>;
        using N = typename G::N;
        using E = typename G::E;
        using COUNT = std::size_t;
        using SLOT_ALLOCATOR=array_container_creator<slot<N, E, COUNT, WEIGHT>, G::node_count(v_node_cnt, w_node_cnt), error_check>;
        using PATH_ALLOCATOR=static_vector_container_creator<E, G::longest_path_edge_count(v_node_cnt, w_node_cnt), error_check>;
        backtracker<G, COUNT, WEIGHT, SLOT_ALLOCATOR, PATH_ALLOCATOR> backtracker_ {};
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
        widenable_to_size_t INDEX,  // C++ won't let you infer this because it relies on v/w's size, qhich aren't known at compile-time
        weight WEIGHT,
        std::size_t V_SIZE,
        std::size_t W_SIZE,
        bool error_check
    >
    auto global_align_stack(
        std::ranges::random_access_range auto&& v,
        std::ranges::random_access_range auto&& w,
        std::function<
            WEIGHT(
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
            )
        > weight_lookup
    ) {
        static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
        static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

        if constexpr (error_check) {
            if (V_SIZE != v.size() || W_SIZE != w.size()) {
                throw std::runtime_error("Size mismatch");
            }
        }
        constexpr INDEX v_node_cnt { V_SIZE + 1zu };
        constexpr INDEX w_node_cnt { W_SIZE + 1zu };
        if constexpr (error_check) {
            if (v_node_cnt != v.size() + 1zu || w_node_cnt != w.size() + 1zu) {
                throw std::runtime_error("Index type too narrow");
            }
        }
        using ND = std::tuple<>;
        using ED = WEIGHT;
        pairwise_global_alignment_graph<
            ND,
            ED,
            INDEX,
            array_grid_container_creator<ND, INDEX, v_node_cnt, w_node_cnt, error_check>,
            array_grid_container_creator<ED, INDEX, v_node_cnt, w_node_cnt, error_check>,
            error_check
        > graph { v_node_cnt, w_node_cnt };
        graph.template assign_weights<WEIGHT>(
            v,
            w,
            weight_lookup,
            [](WEIGHT& edge_data, WEIGHT weight) { edge_data = weight; }
        );
        return align_heap<WEIGHT, error_check>(graph, v, w);
    }

    template<
        widenable_to_size_t INDEX,  // C++ won't let you infer this because it relies on v/w's size, qhich aren't known at compile-time
        weight WEIGHT,
        std::size_t V_SIZE,
        std::size_t W_SIZE,
        bool error_check
    >
    auto local_align_stack(
        std::ranges::random_access_range auto&& v,
        std::ranges::random_access_range auto&& w,
        std::function<
            WEIGHT(
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
            )
        > weight_lookup,
        WEIGHT freeride_weight = {}
    ) {
        static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
        static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

        if constexpr (error_check) {
            if (V_SIZE != v.size() || W_SIZE != w.size()) {
                throw std::runtime_error("Size mismatch");
            }
        }
        constexpr INDEX v_node_cnt { V_SIZE + 1zu };
        constexpr INDEX w_node_cnt { W_SIZE + 1zu };
        if constexpr (error_check) {
            if (v_node_cnt != v.size() + 1zu || w_node_cnt != w.size() + 1zu) {
                throw std::runtime_error("Index type too narrow");
            }
        }
        using ND = std::tuple<>;
        using ED = WEIGHT;
        pairwise_local_alignment_graph<
            ND,
            ED,
            INDEX,
            array_grid_container_creator<ND, INDEX, v_node_cnt, w_node_cnt, error_check>,
            array_grid_container_creator<ED, INDEX, v_node_cnt, w_node_cnt, error_check>,
            error_check
        > graph { v_node_cnt, w_node_cnt };
        graph.template assign_weights<WEIGHT>(
            v,
            w,
            weight_lookup,
            [](WEIGHT& edge_data, WEIGHT weight) { edge_data = weight; },
            freeride_weight
        );
        return align_heap<WEIGHT, error_check>(graph, v, w);
    }

    template<
        widenable_to_size_t INDEX,  // C++ won't let you infer this because it relies on v/w's size, qhich aren't known at compile-time
        weight WEIGHT,
        std::size_t V_SIZE,
        std::size_t W_SIZE,
        bool error_check
    >
    auto fitting_align_stack(
        std::ranges::random_access_range auto&& v,
        std::ranges::random_access_range auto&& w,
        std::function<
            WEIGHT(
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
            )
        > weight_lookup,
        WEIGHT freeride_weight = {}
    ) {
        static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
        static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

        if constexpr (error_check) {
            if (V_SIZE != v.size() || W_SIZE != w.size()) {
                throw std::runtime_error("Size mismatch");
            }
        }
        constexpr INDEX v_node_cnt { V_SIZE + 1zu };
        constexpr INDEX w_node_cnt { W_SIZE + 1zu };
        if constexpr (error_check) {
            if (v_node_cnt != v.size() + 1zu || w_node_cnt != w.size() + 1zu) {
                throw std::runtime_error("Index type too narrow");
            }
        }
        using ND = std::tuple<>;
        using ED = WEIGHT;
        pairwise_fitting_alignment_graph<
            ND,
            ED,
            INDEX,
            array_grid_container_creator<ND, INDEX, v_node_cnt, w_node_cnt, error_check>,
            array_grid_container_creator<ED, INDEX, v_node_cnt, w_node_cnt, error_check>,
            error_check
        > graph { v_node_cnt, w_node_cnt };
        graph.template assign_weights<WEIGHT>(
            v,
            w,
            weight_lookup,
            [](WEIGHT& edge_data, WEIGHT weight) { edge_data = weight; },
            freeride_weight
        );
        return align_heap<WEIGHT, error_check>(graph, v, w);
    }

    template<
        widenable_to_size_t INDEX,  // C++ won't let you infer this because it relies on v/w's size, qhich aren't known at compile-time
        weight WEIGHT,
        std::size_t V_SIZE,
        std::size_t W_SIZE,
        bool error_check
    >
    auto extended_gap_align_stack(
        std::ranges::random_access_range auto&& v,
        std::ranges::random_access_range auto&& w,
        std::function<
            WEIGHT(
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(v[0u])>>>&,
                const std::optional<std::reference_wrapper<const std::remove_reference_t<decltype(w[0u])>>>&
            )
        > weight_lookup,
        WEIGHT gap_weight = {},
        WEIGHT freeride_weight = {}
    ) {
        static_assert(!std::is_rvalue_reference_v<decltype(v)>, "v cannot be an rvalue reference: Function returns references into v, meaning v should continue to exist once function returns.");
        static_assert(!std::is_rvalue_reference_v<decltype(w)>, "w cannot be an rvalue reference: Function returns references into w, meaning w should continue to exist once function returns.");

        if constexpr (error_check) {
            if (V_SIZE != v.size() || W_SIZE != w.size()) {
                throw std::runtime_error("Size mismatch");
            }
        }
        constexpr INDEX v_slot_cnt { V_SIZE + 1zu };
        constexpr INDEX w_slot_cnt { W_SIZE + 1zu };
        if constexpr (error_check) {
            if (v_slot_cnt != v.size() + 1zu || w_slot_cnt != w.size() + 1zu) {
                throw std::runtime_error("Index type too narrow");
            }
        }
        using ND = std::tuple<>;
        using ED = WEIGHT;
        pairwise_extended_gap_alignment_graph<
            ND,
            ED,
            INDEX,
            array_grid_container_creator<
                extended_gap_slot<ND, ED>,
                INDEX,
                v_slot_cnt,
                w_slot_cnt,
                error_check
            >,
            error_check
        > graph { v_slot_cnt, w_slot_cnt };
        graph.template assign_weights<WEIGHT>(
            v,
            w,
            weight_lookup,
            [](WEIGHT& edge_data, WEIGHT weight) { edge_data = weight; },
            gap_weight,
            freeride_weight
        );
        return align_heap<WEIGHT, error_check>(graph, v, w);
    }


    template<std::size_t V_NODE_CNT, std::size_t W_NODE_CNT>
    constexpr auto to_efficient_index_type() {
        constexpr std::size_t n { V_NODE_CNT < W_NODE_CNT ? W_NODE_CNT : V_NODE_CNT };
        if constexpr (n <= std::numeric_limits<std::uint8_t>::max()) {
            return type_encapsulator<std::uint8_t>{};
        } else if constexpr (n <= std::numeric_limits<std::uint16_t>::max()) {
            return type_encapsulator<std::uint16_t>{};
        } else if constexpr (n <= std::numeric_limits<std::uint32_t>::max()) {
            return type_encapsulator<std::uint32_t>{};
        } else if constexpr (n <= std::numeric_limits<std::uint64_t>::max()) {
            return type_encapsulator<std::uint64_t>{};
        } else {
            throw std::runtime_error("Unable to find type large enough to support max number of incoming edges");
        }
    }
}

#endif //OFFBYNULL_ALIGNER_ALIGNER_H
