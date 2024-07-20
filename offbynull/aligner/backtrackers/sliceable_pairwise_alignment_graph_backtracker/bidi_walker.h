#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_H

#include <ranges>
#include <algorithm>
#include <iostream>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/container_creator_packs.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_walker.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs::container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs::heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_walker::sliced_walker;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::utils::static_vector_typer;

    template<
        readable_sliceable_pairwise_alignment_graph G,
        container_creator_pack<G, typename G::ED> CONTAINER_CREATOR_PACK = heap_container_creator_pack<G, typename G::ED, true>,
        bool error_check = true
    >
    requires backtrackable_node<typename G::N> &&
        backtrackable_edge<typename G::E>
    class bidi_walker {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        using SLICE_SLOT_CONTAINER_CREATOR = typename CONTAINER_CREATOR_PACK::SLICE_SLOT_CONTAINER_CREATOR;
        using RESIDENT_SLOT_CONTAINER_CREATOR = typename CONTAINER_CREATOR_PACK::RESIDENT_SLOT_CONTAINER_CREATOR;

        G& g;
        SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator;
        RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator;

    public:
        bidi_walker(
            G& g_,
            SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator = {}
        )
        : g { g_ }
        , slice_slot_container_creator { slice_slot_container_creator }
        , resident_slot_container_creator { resident_slot_container_creator } {}

        std::tuple<ED, std::optional<E>, std::optional<E>> walk_to_node(const N& node) {
            INDEX mid_down_offset {
                std::get<0>(g.node_to_grid_offsets(node))
            };

            prefix_sliceable_pairwise_alignment_graph<G, error_check> prefix_graph {
                g,
                g.slice_last_node(mid_down_offset)
            };
            suffix_sliceable_pairwise_alignment_graph<G, error_check> suffix_graph {
                g,
                g.slice_first_node(mid_down_offset)
            };
            reversed_sliceable_pairwise_alignment_graph<decltype(suffix_graph), error_check> reversed_suffix_graph {
                suffix_graph
            };

            sliced_walker<
                decltype(prefix_graph),
                SLICE_SLOT_CONTAINER_CREATOR,
                RESIDENT_SLOT_CONTAINER_CREATOR,
                error_check
            > forward_walker {
                prefix_graph,
                slice_slot_container_creator,
                resident_slot_container_creator
            };
            while (!forward_walker.next()) {}

            sliced_walker<
                decltype(reversed_suffix_graph),
                SLICE_SLOT_CONTAINER_CREATOR,
                RESIDENT_SLOT_CONTAINER_CREATOR,
                error_check
            > backward_walker {
                reversed_suffix_graph,
                slice_slot_container_creator,
                resident_slot_container_creator
            };
            while (!backward_walker.next()) {}

            const auto& forward_slot { forward_walker.find(node) };
            const auto& backward_slot { backward_walker.find(node) };
            ED weight { forward_slot.backtracking_weight + backward_slot.backtracking_weight };
            std::optional<E> forward_edge { std::nullopt };
            if (g.has_inputs(node)) {
                forward_edge = forward_slot.backtracking_edge;
            }
            std::optional<E> backward_edge { std::nullopt };
            if (g.has_outputs(node)) {
                backward_edge = backward_slot.backtracking_edge;
            }
            return { weight, forward_edge, backward_edge };
        }

        auto backtrack_segmentation_points() {
            ED final_weight {
                std::get<0>(
                    walk_to_node(g.get_leaf_node())
                )
            };

            // constexpr std::size_t container_size {
            //     G::limits(g.grid_down_cnt, g.grid_right_cnt).max_resident_nodes_cnt + 2zu
            // };
            // using CONTAINER = static_vector_typer<N, container_size, error_check>::type;
            using CONTAINER = std::vector<N>;

            CONTAINER container {};
            auto applicable_resident_nodes {
                g.resident_nodes()
                | std::views::transform([this, final_weight](const N& node) {
                    const auto& [weight, forward_edge, backward_edge] { walk_to_node(node) };
                    if (final_weight != weight) {
                        return std::optional<N> { std::nullopt };
                    }
                    if (!g.has_inputs(node)) {  // is root node
                        return std::optional<N> { g.get_edge_to(*backward_edge) };
                    }
                    // is NOT root node (leaf node or any node that isn't root node)
                    return std::optional<N> { g.get_edge_from(*forward_edge) };
                })
                | std::views::filter([](const auto& node_opt) {
                    return node_opt.has_value();
                })
                | std::views::transform([](const auto& node_opt) {
                    N node { *node_opt };
                    return node;
                })
            };
            container.push_back(g.get_root_node());
            container.insert(
                container.end(),
                applicable_resident_nodes.begin(),
                applicable_resident_nodes.end()
            );
            container.push_back(g.get_leaf_node());
            std::ranges::sort(container);

            // container will contain the nodes at which you should cut the graph for divide-and-conquer algorihtm.
            // These are points in that graph where you KNOW resident nodes won't be involved. You can walk using slices
            // only.

            // n1 < n2 if n1 is higher up OR n1 is more to the left OR feeds into n2 (exactly the same grid position but
            // eventually feeds in to n2).

            return std::make_pair(container, final_weight);
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_H
