#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_H

#include <ranges>
#include <algorithm>
#include <iostream>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
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
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slot::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot_with_node;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::helpers::container_creators::static_vector_container_creator;
    using offbynull::utils::static_vector_typer;




    template<typename E>
    struct hop {
        E edge;
    };

    template<typename N>
    struct segment {
        N from_node;
        N to_node;
    };




    template<
        typename T,
        typename G
    >
    concept resident_segmenter_container_creator_pack =
        readable_sliceable_pairwise_alignment_graph<G>
        && bidi_walker_container_creator_pack<typename T::BIDI_WALKER_CONTAINER_CREATOR_PACK, G>
        && container_creator_of_type<typename T::RESIDENT_NODE_CONTAINER_CREATOR, typename G::N>
        && container_creator_of_type<typename T::RESIDENT_EDGE_CONTAINER_CREATOR, typename G::E>
        && container_creator_of_type<typename T::SEGMENT_CONTAINER_CREATOR, std::variant<hop<typename G::E>, segment<typename G::N>>>;

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G
    >
    struct resident_segmenter_heap_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;

        using BIDI_WALKER_CONTAINER_CREATOR_PACK=bidi_walker_heap_container_creator_pack<debug_mode, G>;
        using RESIDENT_NODE_CONTAINER_CREATOR=vector_container_creator<N, debug_mode>;
        using RESIDENT_EDGE_CONTAINER_CREATOR=vector_container_creator<E, debug_mode>;
        using SEGMENT_CONTAINER_CREATOR=vector_container_creator<std::variant<hop<E>, segment<N>>, debug_mode>;
    };

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt
    >
    struct resident_segmenter_stack_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;

        using BIDI_WALKER_CONTAINER_CREATOR_PACK=bidi_walker_stack_container_creator_pack<
            debug_mode,
            G,
            grid_down_cnt,
            grid_right_cnt
        >;
        using RESIDENT_NODE_CONTAINER_CREATOR=static_vector_container_creator<
            N,
            G::limits(grid_down_cnt, grid_right_cnt).max_resident_nodes_cnt,
            debug_mode
        >;
        using RESIDENT_EDGE_CONTAINER_CREATOR=static_vector_container_creator<
            E,
            G::limits(grid_down_cnt, grid_right_cnt).max_resident_nodes_cnt,
            debug_mode
        >;
        using SEGMENT_CONTAINER_CREATOR=static_vector_container_creator<
            std::variant<hop<E>, segment<N>>,
            G::limits(grid_down_cnt, grid_right_cnt).max_path_edge_cnt * 2zu + 1zu,
            debug_mode
        >;
    };






    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        resident_segmenter_container_creator_pack<G> CONTAINER_CREATOR_PACK=resident_segmenter_heap_container_creator_pack<debug_mode, G>
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

        using BIDI_WALKER_CONTAINER_CREATOR_PACK=typename CONTAINER_CREATOR_PACK::BIDI_WALKER_CONTAINER_CREATOR_PACK;
        using RESIDENT_NODE_CONTAINER_CREATOR=typename CONTAINER_CREATOR_PACK::RESIDENT_NODE_CONTAINER_CREATOR;
        using RESIDENT_NODE_CONTAINER=decltype(std::declval<RESIDENT_NODE_CONTAINER_CREATOR>().create_objects(0zu));
        using RESIDENT_EDGE_CONTAINER_CREATOR=typename CONTAINER_CREATOR_PACK::RESIDENT_EDGE_CONTAINER_CREATOR;
        using RESIDENT_EDGE_CONTAINER=decltype(std::declval<RESIDENT_EDGE_CONTAINER_CREATOR>().create_objects(0zu));
        using SEGMENT_CONTAINER_CREATOR=typename CONTAINER_CREATOR_PACK::SEGMENT_CONTAINER_CREATOR;
        using SEGMENT_CONTAINER=decltype(std::declval<SEGMENT_CONTAINER_CREATOR>().create_objects(0zu));

    public:
        resident_segmenter() {}

        auto backtrack_segmentation_points(const G& g, const ED max_path_weight_comparison_tolerance) {
            ED max_path_weight {
                bidi_walker<
                    debug_mode,
                    G,
                    BIDI_WALKER_CONTAINER_CREATOR_PACK
                >::converge_weight(
                    g,
                    g.get_leaf_node()
                )
            };

            const auto& resident_nodes { g.resident_nodes() };
            RESIDENT_NODE_CONTAINER resident_nodes_sorted {
                RESIDENT_NODE_CONTAINER_CREATOR {}.create_copy(resident_nodes)
            };
            std::ranges::sort(resident_nodes_sorted);
            auto resident_nodes_sorted_it { resident_nodes_sorted.begin() };

            RESIDENT_EDGE_CONTAINER resident_edges {
                RESIDENT_EDGE_CONTAINER_CREATOR {}.create_empty(resident_nodes_sorted.size())
            };
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
                            debug_mode,
                            G,
                            BIDI_WALKER_CONTAINER_CREATOR_PACK
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
                    middle_sliceable_pairwise_alignment_graph<debug_mode, G> sub_graph {
                        g,
                        last_to_node,
                        g.get_leaf_node()
                    };
                    const auto& converged_edges_at_resident_node {
                        bidi_walker<
                            debug_mode,
                            decltype(sub_graph),
                            BIDI_WALKER_CONTAINER_CREATOR_PACK
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

            SEGMENT_CONTAINER parts {
                SEGMENT_CONTAINER_CREATOR {}.create_empty(resident_edges.size() * 2zu + 1zu)
            };
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
