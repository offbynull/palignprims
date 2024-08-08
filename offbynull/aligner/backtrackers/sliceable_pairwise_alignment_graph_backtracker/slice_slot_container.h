#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICE_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICE_SLOT_CONTAINER_H

#include <functional>
#include <ranges>
#include "offbynull/aligner/concepts.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/slot.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slice_slot_container {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slot::slot;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::concepts::weight;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::vector_container_creator;

    template<
        readable_sliceable_pairwise_alignment_graph G,
        container_creator CONTAINER_CREATOR=vector_container_creator<slot<typename G::E, typename G::ED>>,
        bool error_check = true
    >
    class slice_slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        using CONTAINER=decltype(std::declval<CONTAINER_CREATOR>().create_objects(0zu));

        const G& graph;
        CONTAINER slots;
        INDEX grid_down;

    public:
        slice_slot_container(
            const G& graph_,
            const CONTAINER_CREATOR container_creator
        )
        : graph{graph_}
        , slots{
            container_creator.create_objects(
                graph_.grid_right_cnt * G::limits(graph_.grid_down_cnt, graph_.grid_right_cnt).max_grid_node_depth
            )
        }
        , grid_down{} {}

        std::optional<std::reference_wrapper<slot<E, ED>>> find(const N& node) {
            const auto& [down_offset, right_offset, depth] { graph.node_to_grid_offsets(node) };
            if (grid_down != down_offset) {
                return { std::nullopt };
            }
            std::size_t idx {
                (
                    right_offset
                    * G::limits(
                        graph.grid_down_cnt,
                        graph.grid_right_cnt
                    ).max_grid_node_depth
                ) + depth
            };
            return { { slots[idx] } };
        }

        void reset(INDEX grid_down_) {
            this->grid_down = grid_down_;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_SLICE_SLOT_CONTAINER_H
