#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_H

#include <cstddef>
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/utils.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container {
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::aligner::graph::pairwise_alignment_graph::readable_pairwise_alignment_graph;

    template<typename N, typename E, weight ED>
    struct slot {
        N node;
        std::size_t unwalked_parent_cnt;
        E backtracking_edge;
        ED backtracking_weight;

        slot(N node_, std::size_t unwalked_parent_cnt_)
        : node{node_}
        , unwalked_parent_cnt{unwalked_parent_cnt_}
        , backtracking_edge{}
        , backtracking_weight{} {}

        slot()
        : node{}
        , unwalked_parent_cnt{}
        , backtracking_edge{}
        , backtracking_weight{} {}
    };






    template<
        typename T,
        typename N,
        typename E,
        typename ED
    >
    concept slot_container_container_creator_pack =
        weight<ED>
        && requires(const T t, std::size_t grid_down_cnt, std::size_t grid_right_cnt, std::size_t grid_depth_cnt) {
            { t.create_slot_container(grid_down_cnt, grid_right_cnt, grid_depth_cnt) } -> random_access_range_of_type<slot<N, E, ED>>;
        };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight ED
    >
    struct slot_container_heap_container_creator_pack {
        std::vector<slot<N, E, ED>> create_slot_container(std::size_t grid_down_cnt, std::size_t grid_right_cnt, std::size_t grid_depth_cnt) const {
            std::size_t cnt { (grid_down_cnt * grid_right_cnt) * grid_depth_cnt };
            return std::vector<slot<N, E, ED>>(cnt);
        }
    };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight ED,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt
    >
    struct slot_container_stack_container_creator_pack {
        static constexpr std::size_t ELEM_COUNT { grid_down_cnt * grid_right_cnt * grid_depth_cnt };

        std::array<slot<N, E, ED>, ELEM_COUNT> create_slot_container(std::size_t grid_down_cnt_, std::size_t grid_right_cnt_, std::size_t grid_depth_cnt_) const {
            if constexpr (debug_mode) {
                std::size_t cnt { (grid_down_cnt_ * grid_right_cnt_) * grid_depth_cnt_ };
                if (cnt != ELEM_COUNT) {
                    throw std::runtime_error("Bad element count");
                }
            }
            return std::array<slot<N, E, ED>, ELEM_COUNT> {};
        }
    };





    template<
        bool debug_mode,
        readable_pairwise_alignment_graph G,
        slot_container_container_creator_pack<typename G::N, typename G::E, typename G::ED> CONTAINER_CREATOR_PACK=slot_container_heap_container_creator_pack<debug_mode, typename G::N, typename G::E, typename G::ED>
    >
    class slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        using SLOT_CONTAINER=decltype(std::declval<CONTAINER_CREATOR_PACK>().create_slot_container(0zu, 0zu, 0zu));

        const G& g;
        SLOT_CONTAINER slots;

    public:
        slot_container(
            const G& g_,
            /*input_iterator_of_type<slot<N, E, WEIGHT>>*/ auto begin,
            /*std::sentinel_for<decltype(begin)>*/ auto end,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : g { g_ }
        , slots {
            container_creator_pack.create_slot_container(
                g.grid_down_cnt,
                g.grid_right_cnt,
                g.grid_depth_cnt
            )
        } {
            auto it { begin };
            while (it != end) {
                const auto& slot { *it };
                const auto& [down_offset, right_offset, depth] { g.node_to_grid_offsets(slot.node) };
                std::size_t idx { (g.grid_depth_cnt * ((down_offset * g.grid_right_cnt) + right_offset)) + depth };
                slots[idx] = slot;
                ++it;
            }
        }

        std::size_t find_idx(const N& node){
            const auto& [down_offset, right_offset, depth] { g.node_to_grid_offsets(node) };
            return (g.grid_depth_cnt * ((down_offset * g.grid_right_cnt) + right_offset)) + depth;
        }

        slot<N, E, ED>& find_ref(const N& node) {
            std::size_t idx { find_idx(node) };
            slot<N, E, ED>& slot { slots[idx] };
            return slot;
        }

        slot<N, E, ED>& at_idx(const std::size_t idx) {
            return slots[idx];
        }

        std::pair<std::size_t, slot<N, E, ED>&> find(const N& node) {
            std::size_t idx { find_idx(node) };
            slot<N, E, ED>& slot { slots[idx] };
            return { idx, slot };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_H
