#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_H

#include <cstddef>
#include <vector>
#include <utility>
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::ready_queue {
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::concepts::unqualified_value_type;
    using offbynull::utils::static_vector_typer;




    template<
        typename T
    >
    concept ready_queue_container_creator_pack =
        unqualified_value_type<T>
        && requires(const T t) {
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
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt
    >
    struct ready_queue_stack_container_creator_pack {
        static constexpr std::size_t ELEM_COUNT { grid_down_cnt * grid_right_cnt * grid_depth_cnt };
        using CONTAINER_TYPE = typename static_vector_typer<debug_mode, std::size_t, ELEM_COUNT>::type;
        CONTAINER_TYPE create_queue_container() const {
            return {};
        }
    };
    
    
    
    
    template<
        bool debug_mode,
        readable_pairwise_alignment_graph G,
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
#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_READY_QUEUE_H
