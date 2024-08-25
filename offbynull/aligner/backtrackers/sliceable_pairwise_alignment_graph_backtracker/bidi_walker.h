#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_H

#include <cstddef>
#include <utility>
#include <ranges>
#include <cmath>
#include <stdexcept>
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker
        ::forward_walker_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker
        ::forward_walker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker
        ::forward_walker_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slot::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_with_node;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::utils::static_vector_typer;




    template<
        typename T,
        typename N,
        typename E,
        typename ED
    >
    concept bidi_walker_container_creator_pack =
        weight<ED>
        && requires(T t) {
            { t.create_forward_walker_container_creator_pack() } -> forward_walker_container_creator_pack<N, E, ED>;
            { t.create_backward_walker_container_creator_pack() } -> forward_walker_container_creator_pack<N, E, ED>;
        };

    template<
        bool debug_mode,
        typename N,
        typename E,
        typename ED
    >
    struct bidi_walker_heap_container_creator_pack {
        forward_walker_heap_container_creator_pack<debug_mode, N, E, ED> create_forward_walker_container_creator_pack() {
            return {};
        }

        forward_walker_heap_container_creator_pack<debug_mode, N, E, ED> create_backward_walker_container_creator_pack() {
            return {};
        }
    };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight ED,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt,
        std::size_t resident_nodes_capacity
    >
    struct bidi_walker_stack_container_creator_pack {
        forward_walker_stack_container_creator_pack<
            debug_mode,
            N,
            E,
            ED,
            grid_right_cnt,
            grid_depth_cnt,
            resident_nodes_capacity
        > create_forward_walker_container_creator_pack() {
            return {};
        }

        forward_walker_stack_container_creator_pack<
            debug_mode,
            N,
            E,
            ED,
            grid_right_cnt,
            grid_depth_cnt,
             resident_nodes_capacity
        > create_backward_walker_container_creator_pack() {
            return {};
        }
    };




    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        bidi_walker_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = bidi_walker_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            typename G::ED
        >
    >
    requires backtrackable_node<typename G::N> &&
        backtrackable_edge<typename G::E>
    class bidi_walker {
    public:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

    private:
        using FORWARD_WALKER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_forward_walker_container_creator_pack());
        using BACKWARD_WALKER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_backward_walker_container_creator_pack());

        const G& g;
        const INDEX target_slice;
        const reversed_sliceable_pairwise_alignment_graph<debug_mode, G> reversed_g;
        forward_walker<debug_mode, G, FORWARD_WALKER_CONTAINER_CREATOR_PACK> forward_walker_;
        forward_walker<debug_mode, decltype(reversed_g), BACKWARD_WALKER_CONTAINER_CREATOR_PACK> backward_walker;

    public:
        static bidi_walker create_and_initialize(
            const G& g_,
            const INDEX target_slice,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        ) {
            if constexpr (debug_mode) {
                if (target_slice >= g_.grid_down_cnt) {
                    throw std::runtime_error("Slice too far down");
                }
            }
            bidi_walker ret {
                g_,
                target_slice,
                container_creator_pack
            };
            return ret;
        }

        struct find_result {
            const slot<E, ED>& forward_slot;
            const slot<E, ED>& backward_slot;
        };

        find_result find(const N& node) {
            const auto& forward_slot { forward_walker_.find(node) };
            const auto& backward_slot { backward_walker.find(node) };
            return { forward_slot, backward_slot };
        }

        struct list_entry {
            N node;
            find_result slots;
        };

        auto list() {
            return g.slice_nodes(target_slice)
                | std::views::transform([&](const N& n) {
                    return list_entry { n, find(n) };
                });
        }

        // Equivalent to find_result, but slots are NOT REFERENCES into the bidi_walker. That way, if the bidi_walker goes out of scope,
        // it won't be pointing to released memory.
        struct find_result_copy {
            const slot<E, ED> forward_slot;
            const slot<E, ED> backward_slot;
        };

        static find_result_copy converge(
            const G& g,
            const N& node
        ) {
            const auto& [down, right, depth] { g.node_to_grid_offsets(node) };
            bidi_walker bidi_walker_ { bidi_walker::create_and_initialize(g, down) };
            find_result found { bidi_walker_.find(node) };
            return find_result_copy { found.forward_slot, found.backward_slot };
        }

        static ED converge_weight(
            const G& g,
            const N& node
        ) {
            find_result_copy slots { converge(g, node) };
            return slots.forward_slot.backtracking_weight + slots.backward_slot.backtracking_weight;
        }

        static bool is_node_on_max_path(
            const G& g_,
            const typename G::N& node,
            const typename G::ED max_path_weight,
            const typename G::ED max_path_weight_comparison_tolerance
        ) {
            const auto& [down, right, depth] { g_.node_to_grid_offsets(node) };

            bidi_walker bidi_walker_ { bidi_walker::create_and_initialize(g_, down) };
            for (const auto& entry : bidi_walker_.list()) {
                ED node_converged_weight { entry.slots.forward_slot.backtracking_weight + entry.slots.backward_slot.backtracking_weight };
                if (std::abs(node_converged_weight -  max_path_weight) <= max_path_weight_comparison_tolerance) {
                    return true;
                }
            }
            return false;
        }

    private:
        bidi_walker(
            const G& g_,
            const INDEX target_slice_,
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : g { g_ }
        , target_slice { target_slice_ }
        , reversed_g { g }
        , forward_walker_ {
            decltype(forward_walker_)::create_and_initialize(
                g,
                target_slice,
                container_creator_pack_.create_forward_walker_container_creator_pack()
            )
        }
        , backward_walker {
            decltype(backward_walker)::create_and_initialize(
                reversed_g,
                g.grid_down_cnt - 1u - target_slice,
                container_creator_pack_.create_backward_walker_container_creator_pack()
            )
        } {}
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_H
