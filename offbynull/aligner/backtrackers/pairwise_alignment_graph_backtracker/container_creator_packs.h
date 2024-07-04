#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONTAINER_CREATOR_PACKS_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONTAINER_CREATOR_PACKS_H

#include <concepts>
#include "offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/slot_container.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/concepts.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::container_creator_packs {
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container::slot;
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::helpers::container_creators::static_vector_container_creator;
    using offbynull::concepts::widenable_to_size_t;

    template<
        typename T,
        typename G,
        typename COUNT,
        typename WEIGHT
    >
    concept container_creator_pack =
        readable_pairwise_alignment_graph<G>
        && widenable_to_size_t<COUNT>
        && weight<WEIGHT>
        && container_creator_of_type<typename T::SLOT_CONTAINER_CREATOR, slot<typename G::N, typename G::E, COUNT, WEIGHT>>
        && container_creator_of_type<typename T::PATH_CONTAINER_CREATOR, typename G::E>;

    template<
        readable_pairwise_alignment_graph G,
        widenable_to_size_t COUNT,
        weight WEIGHT,
        bool error_check = true
    >
    struct heap_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;
        using SLOT_CONTAINER_CREATOR=vector_container_creator<slot<N, E, COUNT, WEIGHT>, error_check>;
        using PATH_CONTAINER_CREATOR=vector_container_creator<E, error_check>;
    };

    template<
        readable_pairwise_alignment_graph G,
        widenable_to_size_t COUNT,
        weight WEIGHT,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt,
        bool error_check = true
    >
    struct stack_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;
        using SLOT_CONTAINER_CREATOR=static_vector_container_creator<
            slot<N, E, COUNT, WEIGHT>,
            G::limits(
                grid_down_cnt,
                grid_right_cnt
            ).max_slice_nodes_cnt,
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

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_CONTAINER_CREATOR_PACKS_H
