#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_RESIDENT_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_RESIDENT_SLOT_CONTAINER_H

#include <functional>
#include <ranges>
#include <algorithm>
#include <vector>
#include <utility>
#include <optional>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_container_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_container_heap_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_with_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_slot_container/resident_slot_with_node_comparator.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_container_creator_pack::resident_slot_container_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_container_heap_container_creator_pack::resident_slot_container_heap_container_creator_pack;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot_with_node
        ::resident_slot_with_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot_with_node_comparator::resident_slot_with_node_comparator;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container
        ::resident_slot::resident_slot;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::row_slot_container::slot::slot;

    /**
     * Container of
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot::resident_slot "resident slots",
     * used by
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtracker::backtracker to track the
     * backtracking state of nodes within graph which need their backtracking state constantly available.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam G Graph type.
     * @tparam CONTAINER_CREATOR_PACK Container factory type.
     */
    template<
        bool debug_mode,
        sliceable_pairwise_alignment_graph G,
        resident_slot_container_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = resident_slot_container_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            typename G::ED,
            true
        >
    >
    class resident_slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using N_INDEX = typename G::N_INDEX;
        using SLOT_CONTAINER = decltype(
            std::declval<CONTAINER_CREATOR_PACK>().create_slot_container(
                std::declval<std::vector<resident_slot_with_node<N, E, ED>>>()
            )
        );

        SLOT_CONTAINER slots;

    public:
        /**
         * Construct an
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container::resident_slot_container::resident_slot_container
         * instance.
         *
         * @param g Graph.
         * @param zero_weight Initial weight, equivalent to 0 for numeric weights.
         * @param container_creator_pack Container factory.
         */
        resident_slot_container(
            const G& g,
            ED zero_weight,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : slots {
            container_creator_pack.create_slot_container(
                g.resident_nodes()
                | std::views::transform([=](const N& node) {
                    return resident_slot_with_node<N, E, ED> {
                        node,
                        resident_slot<E, ED> {
                            false,  // initialized
                            slot<E, ED> {
                                E {},  // backtracking edge
                                zero_weight
                            }
                        }
                    };
                })
            )
        } {
            std::ranges::sort(slots.begin(), slots.end(), resident_slot_with_node_comparator<N, E, ED> {});
        }

        /**
         * Get reference to slot assigned to some node.
         *
         * @param node Identifier of node to find.
         * @return Reference to slot assigned to `node` if found, otherwise `std::nullopt`.
         */
        std::optional<std::reference_wrapper<resident_slot<E, ED>>> find(const N& node) {
            auto it {
                std::lower_bound(
                    slots.begin(),
                    slots.end(),
                    node,
                    resident_slot_with_node_comparator<N, E, ED> {}
                )
            };
            if (it != slots.end() && (*it).node == node) {
                return { (*it).slot_ };
            }
            return { std::nullopt };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_RESIDENT_SLOT_CONTAINER_H
