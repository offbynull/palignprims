#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONTAINER_CREATOR_PACKS_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONTAINER_CREATOR_PACKS_H

#include <concepts>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_walker.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/path_container.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::container_creator_packs {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::sliced_walker::slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::element;
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::helpers::container_creators::static_vector_container_creator;
    using offbynull::concepts::widenable_to_size_t;

    template<
        typename T,
        typename G,
        typename WEIGHT
    >
    concept container_creator_pack =
        readable_sliceable_pairwise_alignment_graph<G>
        && weight<WEIGHT>
        && container_creator_of_type<typename T::SLICE_SLOT_CONTAINER_CREATOR, slot<typename G::E, WEIGHT>>
        && container_creator_of_type<typename T::RESIDENT_SLOT_CONTAINER_CREATOR, slot<typename G::E, WEIGHT>>
        && container_creator_of_type<typename T::ELEMENT_CONTAINER_CREATOR, element<typename G::E>>
        && container_creator_of_type<typename T::PATH_CONTAINER_CREATOR, typename G::E>;

    template<
        readable_sliceable_pairwise_alignment_graph G,
        weight WEIGHT,
        bool error_check = true
    >
    struct heap_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;
        using SLICE_SLOT_CONTAINER_CREATOR=vector_container_creator<slot<E, WEIGHT>, error_check>;
        using RESIDENT_SLOT_CONTAINER_CREATOR=vector_container_creator<slot<E, WEIGHT>, error_check>;
        using ELEMENT_CONTAINER_CREATOR=vector_container_creator<element<E>, error_check>;
        using PATH_CONTAINER_CREATOR=vector_container_creator<E, error_check>;
    };

    template<
        readable_sliceable_pairwise_alignment_graph G,
        weight WEIGHT,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt,
        bool error_check = true
    >
    struct stack_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;
        using SLICE_SLOT_CONTAINER_CREATOR=static_vector_container_creator<
            slot<E, WEIGHT>,
            G::limits(
                grid_down_cnt,
                grid_right_cnt
            ).max_slice_nodes_cnt,
            error_check
        >;
        using RESIDENT_SLOT_CONTAINER_CREATOR=static_vector_container_creator<
            slot<E, WEIGHT>,
            G::limits(
                grid_down_cnt,
                grid_right_cnt
            ).max_resident_nodes_cnt,
            error_check
        >;
        using ELEMENT_CONTAINER_CREATOR=static_vector_container_creator<
            element<E>,
            G::limits(
                grid_down_cnt,
                grid_right_cnt
            ).max_path_edge_cnt,
            error_check
        >;
        using PATH_CONTAINER_CREATOR=static_vector_container_creator<
            E,
            G::limits(
                grid_down_cnt,
                grid_right_cnt
            ).max_path_edge_cnt,
            error_check
        >;
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONTAINER_CREATOR_PACKS_H
