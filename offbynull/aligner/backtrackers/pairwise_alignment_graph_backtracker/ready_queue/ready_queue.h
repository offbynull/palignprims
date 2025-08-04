#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_H

#include <cstddef>
#include <utility>
#include <limits>
#include <stdexcept>
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/ready_queue/ready_queue_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/ready_queue/ready_queue_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/ready_queue/ready_queue_stack_container_creator_pack.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue {
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue_container_creator_pack
        ::ready_queue_container_creator_pack;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue_heap_container_creator_pack
        ::ready_queue_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue_stack_container_creator_pack
        ::ready_queue_stack_container_creator_pack;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::utils::static_vector_typer;

    /**
     * Queue of node positions within an
     * @ref offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph::readable_pairwise_alignment_graph,
     * referred to as a "ready queue" because it's used by the
     * @link offbynull::aligner:backtrackers::pairwise_alignment_graph_backtracker::backtracker::backtracker backtracking algorithm @endlink
     * to track which nodes withing a graph are ready for processing (a node is ready for processing once all of its parents have been
     * processed).
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam SLOT_INDEX Slot indexer type. Must be wide enough to hold the value `grid_down_cnt * grid_right_cnt * grid_depth_cnt`
     *     (variables being multiplied are the dimensions of the `G` instance).
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        readable_pairwise_alignment_graph G,
        widenable_to_size_t SLOT_INDEX,
        ready_queue_container_creator_pack<
            SLOT_INDEX
        > CONTAINER_CREATOR_PACK = ready_queue_heap_container_creator_pack<
            debug_mode,
            SLOT_INDEX,
            true
        >
    >
    class ready_queue {
    private:
        using QUEUE_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_queue_container(0zu, 0zu, 0zu));

        QUEUE_CONTAINER queue;

    public:
        /**
         * Construct an @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue::ready_queue::ready_queue
         * instance.
         *
         * @param g Pairwise alignment graph whose nodes are to be tracked.
         * @param container_creator_pack Container factory.
         */
        ready_queue(
            const G& g,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : queue { container_creator_pack.create_queue_container(g.grid_down_cnt, g.grid_right_cnt, g.grid_depth_cnt) } {
            if constexpr (debug_mode) {
                std::size_t max_size { g.grid_down_cnt * g.grid_right_cnt * g.grid_depth_cnt };
                if (std::numeric_limits<SLOT_INDEX>::max() <= max_size) {
                    throw std::runtime_error { "Type not wide enough" };
                }
            }
        }

        /**
         * Test if this queue is empty.
         *
         * @return `true` if empty, `false` otherwise.
         */
        bool empty() const {
            return queue.empty();
        }

        /**
         * Push node index (index that node resides in within the backtracker's container). A node should only be pushed once it's ready for
         * processing (once all its parents have been processed).
         *
         * @param idx Node index.
         */
        void push(SLOT_INDEX idx) {
            queue.push_back(idx);
        }

        /**
         * Pop node index. If this queue is empty, this function's behavior is undefined.
         *
         * @return Node index.
         */
        SLOT_INDEX pop() {
            auto ret { queue.back() };
            queue.pop_back();
            return ret;
        }
    };
}
#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_READY_QUEUE_H
