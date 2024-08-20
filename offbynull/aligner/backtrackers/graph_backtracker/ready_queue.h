#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_H

#include <cstddef>
#include <boost/container/small_vector.hpp>
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::ready_queue {
    using offbynull::aligner::graph::graph::readable_graph;
    using offbynull::concepts::random_access_range_of_type;




    template<
        typename T
    >
    concept ready_queue_container_creator_pack =
        requires(const T t) {
            { t.create_queue_container() } -> random_access_range_of_type<std::size_t>;
        };

    template<
        bool debug_mode
    >
    struct ready_queue_heap_container_creator_pack {
        std::vector<std::size_t> create_queue_container() const {
            return {};
        }
    };

    template<
        bool debug_mode,
        std::size_t heap_escape_size = 100zu
    >
    struct ready_queue_stack_container_creator_pack {
        boost::container::small_vector<
            std::size_t,
            heap_escape_size
        > create_queue_container() const {
            return {};
        }
    };




    template<
        bool debug_mode,
        readable_graph G,
        ready_queue_container_creator_pack CONTAINER_CREATOR_PACK = ready_queue_heap_container_creator_pack<debug_mode>
    >
    class ready_queue {
    private:
        using QUEUE_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_queue_container());

        QUEUE_CONTAINER queue;

    public:
        ready_queue(
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : queue { container_creator_pack.create_queue_container() } {}

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
