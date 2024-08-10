#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLOT_CONTAINER_H

#include <cstddef>
#include <functional>
#include "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/utils.h"
#include "offbynull/aligner/graph/pairwise_alignment_graph.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::slot_container {
    using offbynull::aligner::concepts::weight;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::helpers::container_creators::array_container_creator;
    using offbynull::concepts::input_iterator_of_type;
    using offbynull::concepts::widenable_to_size_t;
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
        typename G
    >
    concept slot_container_container_creator_pack =
    readable_pairwise_alignment_graph<G>
    && container_creator_of_type<typename T::SLOT_CONTAINER_CREATOR, slot<typename G::N, typename G::E, typename G::ED>>;

    template<
        bool error_check,
        readable_pairwise_alignment_graph G
    >
    struct slot_container_heap_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        using SLOT_CONTAINER_CREATOR=vector_container_creator<slot<N, E, ED>, error_check>;
    };

    template<
        bool error_check,
        readable_pairwise_alignment_graph G,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt
    >
    struct slot_container_stack_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        using SLOT_CONTAINER_CREATOR=array_container_creator<
            slot<N, E, ED>,
            grid_down_cnt * grid_right_cnt * G::limits(grid_down_cnt, grid_right_cnt).max_grid_node_depth,
            error_check
        >;
    };





    template<
        bool error_check,
        readable_pairwise_alignment_graph G,
        slot_container_container_creator_pack<G> CONTAINER_CREATOR_PACK=slot_container_heap_container_creator_pack<error_check, G>
    >
    class slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;

        using SLOT_CONTAINER_CREATOR=typename CONTAINER_CREATOR_PACK::SLOT_CONTAINER_CREATOR;
        using SLOT_CONTAINER=decltype(std::declval<SLOT_CONTAINER_CREATOR>().create_objects(0zu));

        const G& g;
        SLOT_CONTAINER slots;

    public:
        slot_container(
            const G& g_,
            /*input_iterator_of_type<slot<N, E, WEIGHT>>*/ auto begin,
            /*std::sentinel_for<decltype(begin)>*/ auto end
        )
        : g { g_ }
        , slots {
            SLOT_CONTAINER_CREATOR {}.create_objects(
                (g.grid_down_cnt * g.grid_right_cnt) *
                G::limits(g.grid_down_cnt, g.grid_right_cnt).max_grid_node_depth
            )
        } {
            auto it { begin };
            while (it != end) {
                const auto& slot { *it };
                const auto& [down_offset, right_offset, depth] { g.node_to_grid_offsets(slot.node) };
                const auto& max_grid_node_depth { G::limits(g.grid_down_cnt, g.grid_right_cnt).max_grid_node_depth };
                std::size_t idx { (max_grid_node_depth * ((down_offset * g.grid_right_cnt) + right_offset)) + depth };
                slots[idx] = slot;
                ++it;
            }
        }

        std::size_t find_idx(const N& node){
            const auto& [down_offset, right_offset, depth] { g.node_to_grid_offsets(node) };
            const auto& max_grid_node_depth { G::limits(g.grid_down_cnt, g.grid_right_cnt).max_grid_node_depth };
            return (max_grid_node_depth * ((down_offset * g.grid_right_cnt) + right_offset)) + depth;
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
