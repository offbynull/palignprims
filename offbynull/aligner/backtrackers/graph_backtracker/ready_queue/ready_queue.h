#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_H

#include <cstddef>
#include <utility>
#include <stdexcept>
#include "offbynull/aligner/backtrackers/graph_backtracker/ready_queue/ready_queue_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/ready_queue/ready_queue_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/graph_backtracker/ready_queue/ready_queue_stack_container_creator_pack.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue {
    using offbynull::aligner::graph::graph::graph;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue_container_creator_pack
        ::ready_queue_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue_heap_container_creator_pack
        ::ready_queue_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue_stack_container_creator_pack
        ::ready_queue_stack_container_creator_pack;

    /**
     * Queue of node positions within an
     * @ref offbynull::aligner::graph::pairwise_alignment_graph::pairwise_alignment_graph::pairwise_alignment_graph,
     * referred to as a "ready queue" because it's used by
     * @ref offbynull::aligner::backtrackers::graph_backtracker::backtracker::backtracker "backtracking algorithm" to track which nodes
     * within a graph are ready for processing (a node is ready for processing once all of its parents have been processed).
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        graph G,
        ready_queue_container_creator_pack CONTAINER_CREATOR_PACK = ready_queue_heap_container_creator_pack<debug_mode>
    >
    class ready_queue {
    private:
        using QUEUE_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_queue_container());

        QUEUE_CONTAINER queue;

    public:
        /**
         * Construct an @ref offbynull::aligner::backtrackers::graph_backtracker::ready_queue::ready_queue::ready_queue instance.
         *
         * @param container_creator_pack Container factory.
         */
        ready_queue(
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : queue { container_creator_pack.create_queue_container() } {}

        /**
         * Test if this queue is empty.
         *
         * @return `true` if empty, `false` otherwise.
         */
        bool empty() {
            return queue.empty();
        }

        /**
         * Push node index (index that node resides in within the backtracker's container). A node should only be pushed once it's ready for
         * processing (once all its parents have been processed).
         *
         * @param idx Node index.
         */
        void push(std::size_t idx) {
            queue.push_back(idx);
        }

        /**
         * Pop node index. If this queue is empty, this function's behavior is undefined.
         *
         * @return Node index.
         */
        std::size_t pop() {
            if constexpr (debug_mode) {
                if (queue.empty()) {
                    throw std::runtime_error { "Queue empty" };
                }
            }
            auto ret { queue.back() };
            queue.pop_back();
            return ret;
        }
    };
}
#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_H
