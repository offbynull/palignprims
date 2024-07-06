#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_H

#include <functional>
#include <ranges>
#include <algorithm>
#include "offbynull/aligner/concepts.h"
#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/slot.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slot::slot;
    using offbynull::aligner::concepts::weight;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::vector_container_creator;

    template<typename N, typename E, weight WEIGHT>
    struct node_searchable_slot {
        N node;
        slot<E, WEIGHT> slot_;
    };

    template<typename N, typename E, weight WEIGHT>
    struct node_searchable_slot_comparator {
        bool operator()(
            const node_searchable_slot<N, E, WEIGHT>& lhs,
            const node_searchable_slot<N, E, WEIGHT>& rhs
        ) const noexcept {
            return lhs.node < rhs.node;
        }

        bool operator()(
            const node_searchable_slot<N, E, WEIGHT>& lhs,
            const N& rhs
        ) const noexcept {
            return lhs.node < rhs;
        }

        bool operator()(
            const N& lhs,
            const node_searchable_slot<N, E, WEIGHT>& rhs
        ) const noexcept {
            return lhs < rhs.node;
        }
    };

    template<
        readable_sliceable_pairwise_alignment_graph G,
        weight WEIGHT,
        container_creator CONTAINER_CREATOR=vector_container_creator<
            node_searchable_slot<
                typename G::N,
                typename G::E,
                WEIGHT
            >
        >,
        bool error_check = true
    >
    class resident_slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        using CONTAINER=decltype(std::declval<CONTAINER_CREATOR>().create_objects(0zu));

        CONTAINER slots;

    public:
        resident_slot_container(
            G& graph_,
            const CONTAINER_CREATOR& container_creator
        )
        : slots{
            container_creator.create_copy(
                graph_.resident_nodes()
                | std::views::transform([](const N& node) {
                    return node_searchable_slot<N, E, WEIGHT> {
                        node,
                        {}
                    };
                })
            )
        } {
            std::ranges::sort(slots.begin(), slots.end(), node_searchable_slot_comparator<N, E, WEIGHT>{});
        }

        std::optional<std::reference_wrapper<slot<E, WEIGHT>>> find(const N& node) {
            auto it {
                std::lower_bound(
                    slots.begin(),
                    slots.end(),
                    node,
                    node_searchable_slot_comparator<N, E, WEIGHT>{}
                )
            };
            if (it != slots.end() && (*it).node == node) {
                return { (*it).slot_ };
            }
            return { std::nullopt };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_H
