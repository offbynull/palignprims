#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_UNIMPLEMENTED_BACKTRACKER_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_UNIMPLEMENTED_BACKTRACKER_CONTAINER_CREATOR_PACK_H

#include <cstddef>

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker
        ::unimplemented_backtracker_container_creator_pack {

    /**
     * Unimplemented
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker_container_creator_pack::backtracker_container_creator_pack,
     * intended for documentation.
     */
    struct unimplemented_backtracker_container_creator_pack {
        /**
         * Create
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter_container_creator_pack::resident_segmenter_container_creator_pack
         * container creator pack.
         *
         * @return
         *     @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter_container_creator_pack::resident_segmenter_container_creator_pack
         *     container creator pack.
         */
        auto create_resident_segmenter_container_creator_pack() const;

        /**
         * Create
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider_container_creator_pack::sliced_subdivider_container_creator_pack
         * container creator pack.
         *
         * @return
         *     @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_subdivider::sliced_subdivider_container_creator_pack::sliced_subdivider_container_creator_pack
         *     container creator pack.
         */
        auto create_sliced_subdivider_container_creator_pack() const;

        /**
         * Create
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container_container_creator_pack::path_container_container_creator_pack
         * container creator pack.
         *
         * @param path_edge_capacity Maximum number of edges the container can hold.
         * @return
         *     @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container_container_creator_pack::path_container_container_creator_pack
         *     container creator pack.
         */
        auto create_path_container(std::size_t path_edge_capacity) const;
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_UNIMPLEMENTED_BACKTRACKER_CONTAINER_CREATOR_PACK_H