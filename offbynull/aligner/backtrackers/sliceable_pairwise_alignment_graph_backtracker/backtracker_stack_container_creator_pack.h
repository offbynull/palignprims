#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_STACK_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_STACK_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include <stdexcept>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/path_container/path_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter/resident_segmenter_stack_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider/sliced_subdivider_stack_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/unimplemented_backtracker_container_creator_pack.h"
#include "offbynull/aligner/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker_stack_container_creator_pack {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter
        ::resident_segmenter_stack_container_creator_pack::resident_segmenter_stack_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider
        ::sliced_subdivider_stack_container_creator_pack::sliced_subdivider_stack_container_creator_pack;
    using offbynull::utils::static_vector_typer;

    /**
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker_container_creator_pack::backtracker_container_creator_pack
     * that allocates its containers on the stack.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N Graph node identifier type.
     * @tparam E Graph edge identifier type.
     * @tparam ED Graph edge data type (edge weight).
     * @tparam grid_right_cnt Expected right dimension of the underlying pairwise alignment graph instance.
     * @tparam grid_depth_cnt Expected depth dimension of the underlying pairwise alignment graph instance.
     * @tparam resident_nodes_capacity Of all nodes within the underlying pairwise alignment graph, the maximum number of nodes that are
     *     resident nodes (can be higher than the maximum, but not lower).
     * @tparam path_edge_capacity Of all paths between root and leaf within the underlying pairwise alignment graph, the maximum number of
     *     edges.
     */
    template<
        bool debug_mode,
        backtrackable_node N,
        backtrackable_edge E,
        weight ED,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt,
        std::size_t resident_nodes_capacity,
        std::size_t path_edge_capacity
    >
    struct backtracker_stack_container_creator_pack {
        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::unimplemented_backtracker_container_creator_pack::unimplemented_backtracker_container_creator_pack::create_resident_segmenter_container_creator_pack
         */
        resident_segmenter_stack_container_creator_pack<
            debug_mode,
            N,
            E,
            ED,
            grid_right_cnt,
            grid_depth_cnt,
            resident_nodes_capacity
        > create_resident_segmenter_container_creator_pack() const {
            return {};
        }

        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::unimplemented_backtracker_container_creator_pack::unimplemented_backtracker_container_creator_pack::create_sliced_subdivider_container_creator_pack
         */
        sliced_subdivider_stack_container_creator_pack<
            debug_mode,
            N,
            E,
            ED,
            grid_right_cnt,
            grid_depth_cnt,
            path_edge_capacity
        > create_sliced_subdivider_container_creator_pack() const {
            return {};
        }

        /** `create_path_container()` return type. */
        using PATH_CONTAINER_TYPE = typename static_vector_typer<debug_mode, E, path_edge_capacity>::type;

        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::unimplemented_backtracker_container_creator_pack::unimplemented_backtracker_container_creator_pack::create_path_container
         */
        PATH_CONTAINER_TYPE create_path_container(std::size_t path_edge_capacity_) const {
            if constexpr (debug_mode) {
                if (path_edge_capacity_ > path_edge_capacity) {
                    throw std::runtime_error { "Path edge capacity too large" };
                }
            }
            return {};
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_BACKTRACKER_STACK_CONTAINER_CREATOR_PACK_H
