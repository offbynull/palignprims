#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_H

#include <cstddef>
#include <stdexcept>
#include "offbynull/helpers/container_creators.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::ready_queue {
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::helpers::container_creators::small_vector_container_creator;
    using offbynull::aligner::graph::graph::readable_graph;




    template<
        typename T,
        typename G
    >
    concept ready_queue_container_creator_pack =
    readable_graph<G>
    && container_creator_of_type<typename T::QUEUE_CONTAINER_CREATOR, std::size_t>;

    template<
        bool debug_mode,
        readable_graph G
    >
    struct ready_queue_heap_container_creator_pack {
        using QUEUE_CONTAINER_CREATOR=vector_container_creator<std::size_t, debug_mode>;
    };

    template<
        bool debug_mode,
        readable_graph G,
        std::size_t heap_escape_size = 100zu
    >
    struct ready_queue_stack_container_creator_pack {
        using SLOT_CONTAINER_CREATOR=small_vector_container_creator<
            std::size_t,
            heap_escape_size,
            debug_mode
        >;
    };




    template<
        bool debug_mode,
        readable_graph G,
        ready_queue_container_creator_pack<G> CONTAINER_CREATOR_PACK=ready_queue_heap_container_creator_pack<debug_mode, G>
    >
    class ready_queue {
    private:
        using QUEUE_CONTAINER_CREATOR=typename CONTAINER_CREATOR_PACK::QUEUE_CONTAINER_CREATOR;
        using QUEUE_CONTAINER=decltype(std::declval<QUEUE_CONTAINER_CREATOR>().create_empty(std::nullopt));

        QUEUE_CONTAINER queue;

    public:
        ready_queue()
        : queue{QUEUE_CONTAINER_CREATOR {}.create_empty(std::nullopt)} {
            if constexpr (debug_mode) {
                if (!queue.empty()) {
                    throw std::runtime_error("Queue must be sized 0 on creation");  // Happens on std::array sized > 0
                }
            }
        }

        bool empty() {
            return queue.empty();
        }

        void push(std::size_t idx) {
            queue.push_back(idx);
        }

        std::size_t pop() {
            auto ret { queue.back() };
            queue.pop_back();
            return ret;
        }
    };
}
#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_H
