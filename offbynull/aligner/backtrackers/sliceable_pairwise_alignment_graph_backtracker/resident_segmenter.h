#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_H

#include <ranges>
#include <algorithm>
#include <iostream>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/container_creator_packs.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs::container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs::heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker;
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
    class resident_segmenter {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

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

        resident_segmenter(
            const SLICE_SLOT_CONTAINER_CREATOR slice_slot_container_creator = {},
            const RESIDENT_SLOT_CONTAINER_CREATOR resident_slot_container_creator = {}
        )
        : slice_slot_container_creator { slice_slot_container_creator }
        , resident_slot_container_creator { resident_slot_container_creator } {}

        auto backtrack_segmentation_points(const G& g, const ED max_path_weight_comparison_tolerance) {
            ED max_path_weight {
                bidi_walker<
                    G,
                    SLICE_SLOT_CONTAINER_CREATOR,
                    RESIDENT_SLOT_CONTAINER_CREATOR,
                    error_check
                >::converge_weight(
                    g,
                    g.get_leaf_node()
                )
            };

            const auto& resident_nodes { g.resident_nodes() };
            std::vector<N> resident_nodes_sorted(resident_nodes.begin(), resident_nodes.end());
            std::ranges::sort(resident_nodes_sorted);
            auto resident_nodes_sorted_it { resident_nodes_sorted.begin() };

            std::vector<E> resident_edges {};
            resident_edges.reserve(resident_nodes_sorted.size());
            {
                N last_to_node { g.get_root_node() };
                while (resident_nodes_sorted_it != resident_nodes_sorted.end()) {
                    resident_nodes_sorted_it = std::lower_bound(resident_nodes_sorted_it, resident_nodes_sorted.end(), last_to_node);
                    const N& resident_node { *resident_nodes_sorted_it };
                    // Does an optimal path through this resident node? If it doesn't, skip. IS THIS STEP ACTUALLY
                    // NECESSARY?
                    // TODO: Optimize this so that if multiple resident nodes are on the same slice, you don't have to walk the entire graph for each one
                    // TODO: If a resident node is on the path, ignore the ones before it? Start from the resident node >=, if it goes through subdivide and do again?
                    // TODO: More testing need
                    //
                    // TODO: Is there a better way to do this? Find first resident, then instead of looping and walking again, you can shift up/down the existing bidiwalker?
                    // TODO: Is this class even necessary? If you start subdividing from the whole graph, every subdivision should be for a region that the path travels through? IT IS NECESSARY -- THE MIDDLE SLICE OF THE GRAPH MAY BE HOPPED OVER?
                    bool on_max_path {
                        bidi_walker<
                            G,
                            SLICE_SLOT_CONTAINER_CREATOR,
                            RESIDENT_SLOT_CONTAINER_CREATOR,
                            error_check
                        >::is_node_on_max_path(
                            g,
                            resident_node,
                            max_path_weight,
                            max_path_weight_comparison_tolerance
                        )
                    };
                    if (!on_max_path) {
                        ++resident_nodes_sorted_it;
                        continue;
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
                    const auto& converged_edges_at_resident_node {
                        bidi_walker<
                            decltype(sub_graph),
                            SLICE_SLOT_CONTAINER_CREATOR,
                            RESIDENT_SLOT_CONTAINER_CREATOR,
                            error_check
                        >::converge(
                            sub_graph,
                            resident_node
                        )
                    };
                    if (!sub_graph.has_node(resident_node)) { // if node isn't visible, skip
                        ++resident_nodes_sorted_it;
                        continue;
                    }
                    E resident_edge;
                    if (!g.has_inputs(resident_node)) {  // is root node
                        resident_edge = *(converged_edges_at_resident_node.backward_slot).backtracking_edge;
                    } else { // is NOT root node (leaf node or any node that isn't root node)
                        resident_edge = *(converged_edges_at_resident_node.forward_slot).backtracking_edge;
                    }
                    resident_edges.push_back(resident_edge);
                    last_to_node = g.get_edge_to(resident_edge);
                    ++resident_nodes_sorted_it;
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

            return std::make_pair(parts, max_path_weight);
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_H
