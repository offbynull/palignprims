#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_READY_QUEUE_H

#include <cstddef>
#include <boost/container/small_vector.hpp>
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::ready_queue {
    using offbynull::aligner::graph::graph::readable_graph;
    using offbynull::concepts::random_access_range_of_type;




    template<
        typename T,
        typename G
    >
    concept ready_queue_container_creator_pack =
    readable_graph<G>
    && requires(T t) {
        { t.create_queue_container() } -> random_access_range_of_type<std::size_t>;
    };

    template<
        bool debug_mode,
        readable_graph G
    >
    struct ready_queue_heap_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;

        std::vector<std::size_t> create_queue_container() {
            return std::vector<std::size_t> {};
        }
    };

    template<
        bool debug_mode,
        readable_graph G,
        std::size_t heap_escape_size = 100zu
    >
    struct ready_queue_stack_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;

        boost::container::small_vector<std::size_t, heap_escape_size> create_queue_container() {
            return boost::container::small_vector<std::size_t, heap_escape_size> {};
        }
    };




    template<
        bool debug_mode,
        readable_graph G,
        ready_queue_container_creator_pack<G> CONTAINER_CREATOR_PACK=ready_queue_heap_container_creator_pack<debug_mode, G>
    >
    class ready_queue {
    private:
        using QUEUE_CONTAINER=decltype(std::declval<CONTAINER_CREATOR_PACK>().create_queue_container());

        QUEUE_CONTAINER queue;

    public:
        ready_queue(
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : queue{container_creator_pack.create_queue_container()} {}

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
