#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include <iterator>
#include "offbynull/aligner/backtrackers/graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/slot_container_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/slot_container/unimplemented_slot_container_container_creator_pack.h"
#include "offbynull/utils.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_stack_container_creator_pack {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrackers::graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot::slot;
    using offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_container_creator_pack
        ::slot_container_container_creator_pack;
    using offbynull::utils::static_vector_typer;
    using offbynull::concepts::input_iterator_of_non_cvref;

    /**
     * @ref offbynull::aligner::backtrackers::graph_backtracker::slot_container::slot_container_container_creator_pack::slot_container_container_creator_pack
     * that allocates its containers on the heap.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N Graph node type.
     * @tparam E Graph edge type.
     * @tparam WEIGHT Graph edge's weight type.
     * @tparam max_slot_elems For a container created via `create_slot_container()`, the maximum number of slots allowed on the stack.
     */
    template<
        bool debug_mode,
        backtrackable_node N,
        backtrackable_edge E,
        weight WEIGHT,
        std::size_t max_slot_elems
    >
    struct slot_container_stack_container_creator_pack {
        /** `create_slot_container()` return type. */
        using SLOT_CONTAINER_TYPE = typename static_vector_typer<
            debug_mode,
            slot<N, E, WEIGHT>,
            max_slot_elems
        >::type;

        /**
         * @copydoc offbynull::aligner::backtrackers::graph_backtracker::slot_container::unimplemented_slot_container_container_creator_pack::unimplemented_slot_container_container_creator_pack::create_slot_container
         */
        SLOT_CONTAINER_TYPE create_slot_container(
            input_iterator_of_non_cvref<slot<N, E, WEIGHT>> auto begin,
            std::sentinel_for<decltype(begin)> auto end
        ) const {
            return SLOT_CONTAINER_TYPE(begin, end);
        }
    };

    static_assert(
        slot_container_container_creator_pack<slot_container_stack_container_creator_pack<true, int, int, float, 10zu>, int, int, float>
    );
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_SLOT_CONTAINER_STACK_CONTAINER_CREATOR_PACK_H
