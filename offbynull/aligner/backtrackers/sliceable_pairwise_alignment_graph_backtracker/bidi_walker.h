#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_H

#include <ranges>
#include <algorithm>
#include <iostream>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/container_creator_packs.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs::container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs::heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::forward_walker::forward_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slot::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::node_searchable_slot;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::helpers::container_creators::static_vector_container_creator;
    using offbynull::utils::static_vector_typer;

    template<
        readable_sliceable_pairwise_alignment_graph G,
        container_creator SLICE_SLOT_CONTAINER_CREATOR=vector_container_creator<
            slot<
                typename G::E,
                typename G::ED
            >
        >,
        container_creator RESIDENT_SLOT_CONTAINER_CREATOR=vector_container_creator<
            node_searchable_slot<
                typename G::N,
                typename G::E,
                typename G::ED
            >
        >,
        bool error_check = true
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
        const G& g;
        const INDEX target_slice;
        const reversed_sliceable_pairwise_alignment_graph<G, error_check> reversed_g;
        forward_walker<G, SLICE_SLOT_CONTAINER_CREATOR, RESIDENT_SLOT_CONTAINER_CREATOR, error_check> forward_walker_;
        forward_walker<decltype(reversed_g), SLICE_SLOT_CONTAINER_CREATOR, RESIDENT_SLOT_CONTAINER_CREATOR, error_check> backward_walker;
        const SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator;
        const RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator;

    public:
        static bidi_walker create_and_initialize(
            const G& graph,
            const INDEX target_slice,
            const SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            const RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator = {}
        ) {
            if constexpr (error_check) {
                if (target_slice >= graph.grid_down_cnt) {
                    throw std::runtime_error("Slice too far down");
                }
            }
            bidi_walker ret {
                graph,
                target_slice,
                slice_slot_container_creator,
                resident_slot_container_creator
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
                    return list_entry {n, find(n) };
                });
        }

        static find_result converge(
            const G& g,
            const N& node
        ) {
            const auto& [down, right, depth] { g.node_to_grid_offsets(node) };
            bidi_walker bidi_walker_ { bidi_walker::create_and_initialize(g, down) };
            return bidi_walker_.find(node);
        }

        static ED converge_weight(
            const G& g,
            const N& node
        ) {
            find_result slots { converge(g, node) };
            return slots.forward_slot.backtracking_weight + slots.backward_slot.backtracking_weight;
        }

        static bool is_node_on_max_path(
            const G& g,
            const typename G::N& node,
            const typename G::ED max_path_weight,
            const typename G::ED max_path_weight_comparison_tolerance
        ) {
            const auto& [down, right, depth] { g.node_to_grid_offsets(node) };

            bidi_walker bidi_walker_ { bidi_walker::create_and_initialize(g, down) };
            auto list_entries { bidi_walker_.list() };

            auto first_entry { *list_entries.begin() };
            for (const auto& entry : bidi_walker_.list()) {
                N node { entry.node };
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
            const SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            const RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator = {}
        )
        : g { g_ }
        , target_slice { target_slice_ }
        , reversed_g { g }
        , slice_slot_container_creator { slice_slot_container_creator }
        , resident_slot_container_creator { resident_slot_container_creator }
        , forward_walker_ { decltype(forward_walker_)::create_and_initialize(g, target_slice) }
        , backward_walker { decltype(backward_walker)::create_and_initialize(reversed_g, g.grid_down_cnt - 1u - target_slice) } {}
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_H
