#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_RESIDENT_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_RESIDENT_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include <ranges>
#include <stdexcept>
#include <concepts>
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_with_node.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_stack_container_creator_pack {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot_with_node
        ::resident_slot_with_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::forward_range_of_non_cvref;
    using offbynull::utils::static_vector_typer;

    /**
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::row_slot_container_container_creator_pack::row_slot_container_container_creator_pack
     * that allocates its containers on the heap.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N Graph node identifier type.
     * @tparam E Graph edge identifier type.
     * @tparam ED Graph edge data type (edge weight).
     * @tparam resident_nodes_capacity Expected resident nodes capacity of the underlying sliceable pairwise alignment graph instance.
     */
    template<
        bool debug_mode,
        backtrackable_node N,
        backtrackable_edge E,
        weight ED,
        std::size_t resident_nodes_capacity
    >
    struct resident_slot_container_stack_container_creator_pack {
        /**
         * `create_slot_container()` return type.
         */
        using CONTAINER_TYPE = typename static_vector_typer<
            debug_mode,
            resident_slot_with_node<N, E, ED>,
            resident_nodes_capacity
        >::type;

        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::unimplemented_resident_slot_container_container_creator_pack::unimplemented_resident_slot_container_container_creator_pack
         */
        CONTAINER_TYPE create_slot_container(forward_range_of_non_cvref<resident_slot_with_node<N, E, ED>> auto&& r) const  {
            if constexpr (debug_mode) {
                std::integral auto dist { std::ranges::distance(r) };  // This is signed? but resident_nodes_capacity is unsigned?
                if (dist > static_cast<decltype(dist)>(resident_nodes_capacity)) {
                    throw std::runtime_error { "More resident nodes than expected" };
                }
            }
            return CONTAINER_TYPE(r.begin(), r.end());
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_RESIDENT_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H
