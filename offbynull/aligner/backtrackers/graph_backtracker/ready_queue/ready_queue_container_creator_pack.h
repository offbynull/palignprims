#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue_container_creator_pack {
    using offbynull::concepts::random_access_sequence_container;
    using offbynull::concepts::unqualified_object_type;

    /**
     * Concept that's satisfied if `T` has the traits for creating the containers required by
     * @ref offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue::ready_queue, referred to as a container creator
     * pack.
     *
     * @tparam T Type to check.
     */
    template<
        typename T
    >
    concept ready_queue_container_creator_pack =
        unqualified_object_type<T>
        && requires(const T t) {
            { t.create_queue_container() } -> random_access_sequence_container<std::size_t>;
        };
}
#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_CONTAINER_CREATOR_PACK_H
