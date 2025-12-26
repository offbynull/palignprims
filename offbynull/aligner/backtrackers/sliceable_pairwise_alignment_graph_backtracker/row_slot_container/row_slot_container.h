#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_H

#include <functional>
#include <optional>
#include <utility>
#include <cstddef>
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/slot.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/row_slot_container_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/row_slot_container/row_slot_container_heap_container_creator_pack.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::row_slot_container {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container
        ::row_slot_container_container_creator_pack::row_slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container
        ::row_slot_container_heap_container_creator_pack::row_slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot::slot;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::unqualified_object_type;
    using offbynull::utils::static_vector_typer;

    /**
     * Container of @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot "slots",
     * used by
     * @ref offbynull::aligner:backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker to track the
     * backtracking state of each node within a specific row of a graph.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G,
        row_slot_container_container_creator_pack<
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = row_slot_container_heap_container_creator_pack<
            debug_mode,
            typename G::E,
            typename G::ED
        >
    >
    class row_slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using N_INDEX = typename G::N_INDEX;
        using SLOT_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_slot_container(0zu, 0zu));

        const G& g;
        SLOT_CONTAINER slots;
        N_INDEX grid_down;

    public:
        /**
         * Construct an
         * @ref offbynull::aligner::backtrackers::pairwise_alignment_graph_backtracker::row_slot_container::row_slot_container::row_slot_container
         * instance.
         *
         * Defaults to tracking nodes within first row of `g`. See `reset()` to change tracked row.
         *
         * @param g_ Graph.
         * @param container_creator_pack Container factory.
         */
        row_slot_container(
            const G& g_,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : g { g_ }
        , slots {
            container_creator_pack.create_slot_container(
                g.grid_right_cnt,
                g.grid_depth_cnt
            )
        }
        , grid_down {} {}

        // TODO: Is it safe to assume this'll always return something valid? So we can skip the check and std::optional wrapper (or make the
        //       check debug_mode constexpr)

        /**
         * Get reference to slot assigned to some node. Reference becomes invalid once `reset()` invoked.
         *
         * @param node Identifier of node to find.
         * @return Reference to slot assigned to `node` if found, otherwise `std::nullopt`.
         */
        std::optional<std::reference_wrapper<slot<E, ED>>> find(const N& node) {
            const auto& [down_offset, right_offset, depth] { g.node_to_grid_offset(node) };
            if (grid_down != down_offset) {
                return { std::nullopt };
            }
            std::size_t idx { (right_offset * g.grid_depth_cnt) + depth };
            return { { slots[idx] } };
        }

        /**
         * Change tracked row to a different row within the same graph. Once invoked, references to slots returned by `find()` are invalid.
         *
         * If `grid_down_`is out of bounds, the behaviour of this function undefined.
         *
         * @param grid_down_ Row index.
         */
        void reset(N_INDEX grid_down_) {
            this->grid_down = grid_down_;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_ROW_SLOT_CONTAINER_ROW_SLOT_CONTAINER_H
