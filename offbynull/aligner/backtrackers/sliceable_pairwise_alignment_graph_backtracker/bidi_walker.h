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
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        G& g;
        SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator;
        RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator;

    public:
        struct hop {
            E edge;
        };

        struct segment {
            N from_node;
            N to_node;
        };

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

            forward_walker<
                decltype(prefix_graph),
                SLICE_SLOT_CONTAINER_CREATOR,
                RESIDENT_SLOT_CONTAINER_CREATOR,
                error_check
            > forward_walker_ {
                prefix_graph,
                slice_slot_container_creator,
                resident_slot_container_creator
            };
            while (!forward_walker_.next()) {}

            forward_walker<
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

            const auto& forward_slot { forward_walker_.find(node) };
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

            const auto& resident_nodes { g.resident_nodes() };
            std::vector<N> resident_nodes_sorted(resident_nodes.begin(), resident_nodes.end());
            std::ranges::sort(resident_nodes_sorted);

            std::vector<E> resident_edges {};
            resident_edges.reserve(resident_nodes_sorted.size());
            {
                N last_to_node { g.get_root_node() };
                for (const N& resident_node : resident_nodes) {
                    // Does an optimal path path through this resident node? If it doesn't, skip. IS THIS STEP ACTUALLY
                    // NECESSARY?
                    {
                        const auto& [weight, forward_edge, backward_edge] { walk_to_node(resident_node) };
                        if (weight != final_weight) {
                            continue;
                        }
                    }
                    // Isolate graph such that root is last_to_node and walk it instead of full graph. This is required
                    // because, when walking using the whole graph (not isolated), sometimes the edge's "from node" will
                    // be BEFORE last_to_node. Having a "from node" BEFORE last_to_node is a known issue because there
                    // may be more than 1 optimal path and the prior edge vs this edge are targeting different optimal
                    // paths. The problem is that we can't accept that because we're targeting exactly one optimal path,
                    // and so we want to reject anything BEFORE last_to_node because if it goes BEFORE last_to_node then
                    // we know it's targeting a different optimal path vs the one that goes through last_to_node.
                    middle_sliceable_pairwise_alignment_graph<G, error_check> sub_graph {
                        g,
                        last_to_node,
                        g.get_leaf_node()
                    };
                    bidi_walker<
                        decltype(sub_graph),
                        SLICE_SLOT_CONTAINER_CREATOR,
                        RESIDENT_SLOT_CONTAINER_CREATOR
                    > sub_graph_bidi_walker {
                        sub_graph,
                        slice_slot_container_creator,
                        resident_slot_container_creator
                    };
                    const auto& [weight, forward_edge, backward_edge] { sub_graph_bidi_walker.walk_to_node(resident_node) };
                    if (!sub_graph.has_node(resident_node)) { // if node isn't visible, skip
                        continue;
                    }
                    E resident_edge;
                    if (!g.has_inputs(resident_node)) {  // is root node
                        resident_edge = *backward_edge;
                    } else { // is NOT root node (leaf node or any node that isn't root node)
                        resident_edge = *forward_edge;
                    }
                    resident_edges.push_back(resident_edge);
                    last_to_node = g.get_edge_to(resident_edge);
                }
            }

            std::vector<std::variant<hop, segment>> parts {};
            parts.reserve(resident_edges.size() * 2zu + 1zu);
            {
                N last_to_node { g.get_root_node() };
                for (const E& resident_edge : resident_edges) {
                    N from_node { g.get_edge_from(resident_edge) };
                    N to_node { g.get_edge_to(resident_edge) };
                    if (last_to_node != from_node) {
                        parts.push_back({ segment { last_to_node, from_node } });
                    }
                    parts.push_back({ hop { resident_edge } });
                    last_to_node = to_node;
                }
                N final_to_node { g.get_leaf_node() };
                if (last_to_node != final_to_node) {
                    parts.push_back({ segment { last_to_node, final_to_node } });
                }
            }

            // container will contain the nodes at which you should cut the graph for divide-and-conquer algorihtm.
            // These are points in that graph where you KNOW resident nodes won't be involved. You can walk using slices
            // only.

            // n1 < n2 if n1 is higher up OR n1 is more to the left OR feeds into n2 (exactly the same grid position but
            // eventually feeds in to n2).

            return std::make_pair(parts, final_weight);
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BIDI_WALKER_H
