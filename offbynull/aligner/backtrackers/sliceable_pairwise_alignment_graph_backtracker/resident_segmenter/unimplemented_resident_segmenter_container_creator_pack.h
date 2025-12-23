#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_UNIMPLEMENTED_RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_UNIMPLEMENTED_RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter
        ::unimplemented_resident_segmenter_container_creator_pack {

    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::concepts::forward_range_of_non_cvref;

    /**
     * Unimplemented
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter_container_creator_pack::resident_segmenter_container_creator_pack,
     * intended for documentation.
     *
     * @tparam N Graph node type.
     */
    template<
        backtrackable_node N
    >
    struct unimplemented_resident_segmenter_container_creator_pack {
        /**
         * Create
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker_container_creator_pack::bidi_walker_container_creator_pack
         * container creator pack.
         *
         * @return
         *     @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker::bidi_walker_container_creator_pack::bidi_walker_container_creator_pack
         *     container creator pack.
         */
        auto create_bidi_walker_container_creator_pack() const;

        /**
         * Create random access container holding resident nodes.
         *
         * @param resident_nodes Range of resident nodes.
         * @return Random access container containing the nodes within `resident_nodes`.
         */
        auto create_resident_node_container(forward_range_of_non_cvref<N> auto&& resident_nodes) const;

        /**
         * Create random access container used to store edges to/from resident nodes.
         *
         * @param resident_nodes_capacity Maximum number of resident nodes.
         * @return Empty random access container which can hold at least `resident_nodes_capacity` elements.
         */
        auto create_resident_edge_container(std::size_t resident_nodes_capacity) const;

        /**
         * Create random access container used to store a segment-hop chain.
         *
         * @param resident_nodes_capacity Maximum number of elements in the chain, where each element is either a segment or a hop.
         * @return Empty random access container which can hold at least `resident_nodes_capacity` elements.
         */
        auto create_segment_hop_chain_container(std::size_t resident_nodes_capacity) const;
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_UNIMPLEMENTED_RESIDENT_SEGMENTER_CONTAINER_CREATOR_PACK_H