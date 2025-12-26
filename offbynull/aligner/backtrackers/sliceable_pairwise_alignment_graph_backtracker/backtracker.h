#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <utility>
#include <variant>
#include <type_traits>
#include <stdexcept>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker_stack_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter/resident_segmenter.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider/sliced_subdivider.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::hop::hop;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::segment::segment;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter
        ::resident_segmenter;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider
        ::sliced_subdivider::sliced_subdivider;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker_container_creator_pack
        ::backtracker_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker_heap_container_creator_pack
        ::backtracker_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker_stack_container_creator_pack
        ::backtracker_stack_container_creator_pack;
    using offbynull::aligner::graphs::prefix_sliceable_pairwise_alignment_graph::prefix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::suffix_sliceable_pairwise_alignment_graph::suffix_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::middle_sliceable_pairwise_alignment_graph::middle_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::graphs::reversed_sliceable_pairwise_alignment_graph::reversed_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts::backtracking_result;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::concepts
        ::backtracking_result_without_explicit_weight;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::utils::static_vector_typer;

    /**
     * Backtracker for @ref offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph
     * implementations. A backtracker's purpose is to find the maximally-weighted path (path with the highest sum of edge weights) between
     * some directed graph's root node and leaf node, picking an arbitrary one if there are multiple such paths. For a detailed explanation
     * of the backtracking algorithm, see
     * https://offbynull.com/docs/data/learn/Bioinformatics/output/output.html#H_Divide-and-Conquer%20Algorithm.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G,
        backtracker_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = backtracker_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            typename G::ED,
            true
        >
    >
    requires backtrackable_node<typename G::N> &&
        backtrackable_edge<typename G::E>
    class backtracker {
    public:
        /** `G`'s node identifier type. */
        using N = typename G::N;
        /** `G`'s edge identifier type. */
        using E = typename G::E;
        /** `G`'s node data type. */
        using ND = typename G::ND;
        /** `G`'s edge data type. */
        using ED = typename G::ED;
        /** `G`'s grid coordinate type. For example, `std::uint8_t` will allow up to 255 nodes on both the down and right axis. */
        using N_INDEX = typename G::N_INDEX;

        /**
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter::resident_segmenter
         * container factory type used by this backtracker implementation.
         */
        using RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_resident_segmenter_container_creator_pack());
        /**
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider::sliced_subdivider
         * container factory type used by this backtracker implementation.
         */
        using SLICED_SUBDIVIDER_CONTAINER_CREATOR_PACK =
            decltype(std::declval<CONTAINER_CREATOR_PACK>().create_sliced_subdivider_container_creator_pack());
        /**
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container::path_container
         * type used by this backtracker implementation.
         */
        using PATH_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_path_container(0zu));

    private:
        /**
         * Container factory.
         */
        CONTAINER_CREATOR_PACK container_creator_pack;

    public:
        /**
         * Construct an @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker
         * instance.
         *
         * @param container_creator_pack_ Container factory.
         */
        backtracker(
            CONTAINER_CREATOR_PACK container_creator_pack_ = {}
        )
        : container_creator_pack { container_creator_pack_ } {}

        /**
         * Determine the maximally-weighted path (path with the highest sum of edge weights) connecting a sliceable pairwise alignment
         * graph's root node and leaf node.
         *
         * If `max_path_weight_comparison_tolerance` is not a finite value (e.g., NaN or inf), the behavior of this function is
         * undefined.
         *
         * @param g Graph.
         * @param max_path_weight_comparison_tolerance Tolerance used when testing for weight for equality. This may need to be non-zero
         *     when the type used for edge weights is a floating point type (must be finite). It helps mitigate floating point rounding
         *     errors when `g` is large / has large magnitude differences across `g`'s edge weights. The value this should be set to depends
         *     on multiple factors (e.g., which floating point type is used, expected graph size, expected magnitudes, etc..).
         * @return Maximally weighted path from `g`'s root node to `g`'s leaf node, along with that path's weight.
         */
        backtracking_result<ED> auto find_max_path(
            const G& g,
            const ED max_path_weight_comparison_tolerance
        ) {
            if constexpr (debug_mode) {
                if (!std::isfinite(max_path_weight_comparison_tolerance)) {
                    throw std::runtime_error { "Tolerance not finite" };
                }
            }
            resident_segmenter<
                debug_mode,
                G,
                RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK
            > resident_segmenter_ {
                container_creator_pack.create_resident_segmenter_container_creator_pack()
            };
            const auto& [parts, final_weight] {
                resident_segmenter_.backtrack_segmentation_points(g, max_path_weight_comparison_tolerance)
            };
            // NOTE: I spent the better part of a day trying to get parts to stream back instead of putting it in a PATH_CONTAINER and
            //       sending it back. It wasn't doable because in the end I couldn't make the stream copy assignable. The work is in a
            //       branch.
            PATH_CONTAINER path {
                container_creator_pack.create_path_container(
                    g.path_edge_capacity
                )
            };
            for (const auto& part : parts) {
                if (const auto* hop_ptr = std::get_if<hop<E>>(&part)) {
                    path.push_back(hop_ptr->edge);
                } else if (const auto* segment_ptr = std::get_if<segment<N>>(&part)) {
                    middle_sliceable_pairwise_alignment_graph<debug_mode, G> g_segment {
                        g,
                        segment_ptr->from_node,
                        segment_ptr->to_node
                    };
                    sliced_subdivider<
                        debug_mode,
                        decltype(g_segment),
                        SLICED_SUBDIVIDER_CONTAINER_CREATOR_PACK
                    > subdivider {
                        g_segment,
                        container_creator_pack.create_sliced_subdivider_container_creator_pack()
                    };
                    auto path_container { subdivider.subdivide() };
                    for (const E& edge : path_container.walk_path_forward()) {
                        path.push_back(edge);
                    }
                } else {
                    if constexpr (debug_mode) {
                        throw std::runtime_error { "This should never happen" };
                    }
                }
            }
            return std::make_pair(
                std::move(path),
                final_weight
            );  // NOTE: No dangling issues - make_pair() stores values, not refs.
        }
    };

    /**
     * Helper function that constructs an
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker instance utilizing
     * the heap for storage / computations and invokes `find_max_path(g)` on it.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam minimize_allocations Primes certain containers with enough capacity such that adhoc reallocations aren't needed.
     * @param g Graph.
     * @param max_path_weight_comparison_tolerance Tolerance used when testing for weight for equality. This may need to be non-zero when
     *     the type used for edge weights is a floating point type (must be finite). It helps mitigate floating point rounding errors when
     *     `g` is large / has large magnitude differences across `g`'s edge weights. The value this should be set to depends on multiple
     *     factors (e.g., which floating point type is used, expected graph size, expected magnitudes, etc..).
     * @return `find_max_path(g)` result.
     */
    template<
        bool debug_mode,
        bool minimize_allocations
    >
    backtracking_result_without_explicit_weight auto heap_find_max_path(
        const sliceable_pairwise_alignment_graph auto& g,
        typename std::remove_cvref_t<decltype(g)>::ED max_path_weight_comparison_tolerance
    ) {
        using G = std::remove_cvref_t<decltype(g)>;
        return backtracker<
            debug_mode,
            G,
            backtracker_heap_container_creator_pack<
                debug_mode,
                typename G::N,
                typename G::E,
                typename G::ED,
                minimize_allocations
            >
        > {}.find_max_path(g, max_path_weight_comparison_tolerance);
    }

    /**
     * Helper function that constructs an
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker instance utilizing
     * the stack for storage / computations and invokes `find_max_path(g)` on it.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam grid_right_cnt Expected right dimension of the underlying pairwise alignment graph instance.
     * @tparam grid_depth_cnt Expected depth dimension of the underlying pairwise alignment graph instance.
     * @tparam resident_nodes_capacity Of all nodes within the underlying pairwise alignment graph, the maximum number of nodes that are
     *     resident nodes (can be higher than the maximum, but not lower).
     * @tparam path_edge_capacity Of all paths between root and leaf within the underlying pairwise alignment graph, the maximum number of
     *     edges.
     * @param g Graph.
     * @param max_path_weight_comparison_tolerance Tolerance used when testing for weight for equality. This may need to be non-zero when
     *     the type used for edge weights is a floating point type (must be finite). It helps mitigate floating point rounding errors when
     *     `g` is large / has large magnitude differences across `g`'s edge weights. The value this should be set to depends on multiple
     *     factors (e.g., which floating point type is used, expected graph size, expected magnitudes, etc..).
     * @return `find_max_path(g)` result.
     */
    template<
        bool debug_mode,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt,
        std::size_t resident_nodes_capacity,
        std::size_t path_edge_capacity
    >
    backtracking_result_without_explicit_weight auto stack_find_max_path(
        const sliceable_pairwise_alignment_graph auto& g,
        typename std::remove_cvref_t<decltype(g)>::ED max_path_weight_comparison_tolerance
    ) {
        using G = std::remove_cvref_t<decltype(g)>;
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        if constexpr (debug_mode) {
            if (g.grid_right_cnt != grid_right_cnt
                || g.grid_depth_cnt != grid_depth_cnt
                || g.resident_nodes_capacity != resident_nodes_capacity
                || g.path_edge_capacity != path_edge_capacity) {
                throw std::runtime_error { "Unexpected graph dimensions" };
            }
        }
        return backtracker<
            debug_mode,
            G,
            backtracker_stack_container_creator_pack<
                debug_mode,
                N,
                E,
                ED,
                grid_right_cnt,
                grid_depth_cnt,
                resident_nodes_capacity,
                path_edge_capacity
            >
        > {}.find_max_path(g, max_path_weight_comparison_tolerance);
    }
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_H
