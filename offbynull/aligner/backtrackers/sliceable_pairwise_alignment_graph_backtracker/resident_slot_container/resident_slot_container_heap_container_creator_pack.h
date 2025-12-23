#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_RESIDENT_SLOT_CONTAINER_HEAP_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_RESIDENT_SLOT_CONTAINER_HEAP_CONTAINER_CREATOR_PACK_H

#include <vector>
#include <ranges>
#include <type_traits>
#include "offbynull/concepts.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_with_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/unimplemented_resident_slot_container_container_creator_pack.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_heap_container_creator_pack {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot_with_node
        ::resident_slot_with_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::forward_range_of_non_cvref;

    /**
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::row_slot_container_container_creator_pack::row_slot_container_container_creator_pack
     * that allocates its containers on the heap.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N Graph node type.
     * @tparam E Graph edge type.
     * @tparam ED Graph edge data type (edge weight).
     * @tparam minimize_allocations `true` to force `create_slot_container()` to immediately reserve the exact amount of memory needed for
     *     the returned container, thereby removing/reducing the need for adhoc reallocations.
     */
    template<
        bool debug_mode,
        backtrackable_node N,
        backtrackable_edge E,
        weight ED,
        bool minimize_allocations
    >
    struct resident_slot_container_heap_container_creator_pack {
        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::unimplemented_resident_slot_container_container_creator_pack::unimplemented_resident_slot_container_container_creator_pack
         */
        std::vector<resident_slot_with_node<N, E, ED>> create_slot_container(
            forward_range_of_non_cvref<resident_slot_with_node<N, E, ED>> auto&& r
        ) const {
            if constexpr (std::ranges::sized_range<std::remove_cvref_t<decltype(r)>>) {
                std::vector<resident_slot_with_node<N, E, ED>> ret {};
                ret.reserve(std::ranges::size(r));
                for (const auto& e : r) {
                    ret.push_back(e);
                }
                return ret;
            } else if constexpr (minimize_allocations) {
                // Is this a bad idea? Calling std::ranges::distance() on large input much slower vs vector heap resizing that happens?
                std::vector<resident_slot_with_node<N, E, ED>> ret {};
                ret.reserve(std::ranges::distance(r.begin(), r.end()));
                for (const auto& e : r) {
                    ret.push_back(e);
                }
                return ret;
            } else {
                return std::vector<resident_slot_with_node<N, E, ED>>(r.begin(), r.end());
            }
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_RESIDENT_SLOT_CONTAINER_HEAP_CONTAINER_CREATOR_PACK_H
