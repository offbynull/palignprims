#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_H

#include <ranges>
#include <algorithm>
#include <iostream>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"
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
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::random_access_range_of_type;
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
        typename N,
        typename E,
        typename ED
    >
    concept resident_segmenter_container_creator_pack =
        weight<ED>
        && requires(const T t, std::size_t resident_nodes_capacity, const std::vector<N>& resident_nodes) {
            { t.create_bidi_walker_container_creator_pack() } -> bidi_walker_container_creator_pack<N, E, ED>;
            { t.create_resident_node_container(resident_nodes) } -> random_access_range_of_type<N>;
            { t.create_resident_edge_container(resident_nodes_capacity) } -> random_access_range_of_type<E>;
            { t.create_segment_container(resident_nodes_capacity) } -> random_access_range_of_type<std::variant<hop<E>, segment<N>>>;
        };

    template<
        bool debug_mode,
        typename N,
        typename E,
        typename ED
    >
    struct resident_segmenter_heap_container_creator_pack {
        bidi_walker_heap_container_creator_pack<debug_mode, N, E, ED> create_bidi_walker_container_creator_pack() const {
            return {};
        }

        std::vector<N> create_resident_node_container(range_of_type<N> auto&& resident_nodes) const {
            return std::vector<N>(resident_nodes.begin(), resident_nodes.end());
        }

        std::vector<E> create_resident_edge_container(std::size_t resident_nodes_capacity) const {
            std::vector<E> ret {};
            ret.reserve(resident_nodes_capacity);
            return ret;
        }

        std::vector<std::variant<hop<E>, segment<N>>> create_segment_container(std::size_t resident_nodes_capacity) const {
            std::vector<std::variant<hop<E>, segment<N>>> ret {};
            ret.reserve(resident_nodes_capacity * 2zu + 1zu);
            return ret;
        }
    };

    template<
        bool debug_mode,
        typename N,
        typename E,
        typename ED,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt,
        std::size_t resident_nodes_capacity
    >
    struct resident_segmenter_stack_container_creator_pack {
        bidi_walker_stack_container_creator_pack<debug_mode, N, E, ED, grid_right_cnt, grid_depth_cnt, resident_nodes_capacity> create_bidi_walker_container_creator_pack() const {
            return {};
        }

        using RESIDENT_NODE_CONTAINER_TYPE = typename static_vector_typer<N, resident_nodes_capacity, debug_mode>::type;
        RESIDENT_NODE_CONTAINER_TYPE create_resident_node_container(range_of_type<N> auto& resident_nodes) const {
            return RESIDENT_NODE_CONTAINER_TYPE(resident_nodes.begin(), resident_nodes.end());
        }

        using RESIDENT_EDGE_CONTAINER_TYPE = typename static_vector_typer<E, resident_nodes_capacity, debug_mode>::type;
        RESIDENT_EDGE_CONTAINER_TYPE create_resident_edge_container(std::size_t resident_nodes_capacity_) const {
            if constexpr (debug_mode) {
                if (resident_nodes_capacity == resident_nodes_capacity_) {
                    throw std::runtime_error { "Inconsistent node count" };
                }
            }
            return RESIDENT_EDGE_CONTAINER_TYPE {};
        }

        static constexpr std::size_t max_segment_cnt { resident_nodes_capacity * 2zu + 1zu };
        using SEGMENT_CONTAINER_TYPE = typename static_vector_typer<std::variant<hop<E>, segment<N>>, max_segment_cnt, debug_mode>::type;
        SEGMENT_CONTAINER_TYPE create_segment_container(std::size_t resident_nodes_cnt_) const {
            if constexpr (debug_mode) {
                if (resident_nodes_cnt_ * 2zu + 1zu <= max_segment_cnt) {
                    throw std::runtime_error { "Inconsistent node count" };
                }
            }
            return SEGMENT_CONTAINER_TYPE {};
        }
    };






    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        resident_segmenter_container_creator_pack<typename G::N, typename G::E, typename G::ED> CONTAINER_CREATOR_PACK = resident_segmenter_heap_container_creator_pack<debug_mode, typename G::N, typename G::E, typename G::ED>
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

        using BIDI_WALKER_CONTAINER_CREATOR_PACK = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_bidi_walker_container_creator_pack());
        using RESIDENT_NODE_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_resident_node_container(std::declval<std::vector<N>>()));
        using RESIDENT_EDGE_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_resident_edge_container(0zu));
        using SEGMENT_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_segment_container(0zu));

        CONTAINER_CREATOR_PACK container_creator_pack;

    public:
        resident_segmenter(
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : container_creator_pack { container_creator_pack_ } {}

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
                container_creator_pack.create_resident_node_container(
                    resident_nodes
                )
            };
            std::ranges::sort(resident_nodes_sorted);
            auto resident_nodes_sorted_it { resident_nodes_sorted.begin() };

            RESIDENT_EDGE_CONTAINER resident_edges {
                container_creator_pack.create_resident_edge_container(
                    resident_nodes_sorted.size()
                )
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
                container_creator_pack.create_segment_container(
                    resident_edges.size() // * 2zu + 1zu
                )
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
