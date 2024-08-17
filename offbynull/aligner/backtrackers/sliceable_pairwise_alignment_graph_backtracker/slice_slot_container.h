#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICE_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICE_SLOT_CONTAINER_H

#include <functional>
#include <ranges>
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/slot.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slice_slot_container {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slot::slot;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::utils::static_vector_typer;




    template<
        typename T,
        typename E,
        typename ED
    >
    concept slice_slot_container_container_creator_pack =
        weight<ED>
        && requires(const T t, std::size_t grid_right_cnt, std::size_t grid_depth_cnt) {
            { t.create_slot_container(grid_right_cnt, grid_depth_cnt) } -> random_access_range_of_type<slot<E, ED>>;
        };

    template<
        bool debug_mode,
        typename E,
        weight ED
    >
    struct slice_slot_container_heap_container_creator_pack {
        std::vector<slot<E, ED>> create_slot_container(std::size_t grid_right_cnt, std::size_t grid_depth_cnt) const {
            std::size_t cnt { grid_right_cnt * grid_depth_cnt };
            return std::vector<slot<E, ED>>(cnt);
        }
    };

    template<
        bool debug_mode,
        typename E,
        weight ED,
        std::size_t grid_right_cnt,
        std::size_t grid_depth_cnt
    >
    struct slice_slot_container_stack_container_creator_pack {
        static constexpr std::size_t ELEM_COUNT { grid_right_cnt * grid_depth_cnt };

        std::array<slot<E, ED>, ELEM_COUNT> create_slot_container(std::size_t grid_right_cnt_, std::size_t grid_depth_cnt_) const {
            if constexpr (debug_mode) {
                std::size_t cnt { grid_right_cnt_ * grid_depth_cnt_ };
                if (cnt != ELEM_COUNT) {
                    throw std::runtime_error("Bad element count");
                }
            }
            return std::array<slot<E, ED>, ELEM_COUNT> {};
        }
    };




    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        slice_slot_container_container_creator_pack<typename G::E, typename G::ED> CONTAINER_CREATOR_PACK=slice_slot_container_heap_container_creator_pack<debug_mode, typename G::E, typename G::ED>
    >
    class slice_slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        using SLOT_CONTAINER=decltype(std::declval<CONTAINER_CREATOR_PACK>().create_slot_container(0zu, 0zu));

        const G& g;
        SLOT_CONTAINER slots;
        INDEX grid_down;

    public:
        slice_slot_container(
            const G& g_,
            CONTAINER_CREATOR_PACK container_creator_pack={}
        )
        : g{g_}
        , slots{
            container_creator_pack.create_slot_container(
                g.grid_right_cnt,
                g.grid_depth_cnt
            )
        }
        , grid_down{} {}

        std::optional<std::reference_wrapper<slot<E, ED>>> find(const N& node) {
            const auto& [down_offset, right_offset, depth] { g.node_to_grid_offsets(node) };
            if (grid_down != down_offset) {
                return { std::nullopt };
            }
            std::size_t idx { (right_offset * g.grid_depth_cnt) + depth };
            return { { slots[idx] } };
        }

        void reset(INDEX grid_down_) {
            this->grid_down = grid_down_;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICE_SLOT_CONTAINER_H
