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
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::helpers::container_creators::static_vector_container_creator;

    template<typename E, weight WEIGHT>
    struct resident_slot {
        bool initialized;
        slot<E, WEIGHT> slot_;
    };

    template<typename N, typename E, weight WEIGHT>
    struct resident_slot_with_node {
        N node;
        resident_slot<E, WEIGHT> slot_;
    };

    template<typename N, typename E, weight WEIGHT>
    struct resident_slot_with_node_comparator {
        bool operator()(
            const resident_slot_with_node<N, E, WEIGHT>& lhs,
            const resident_slot_with_node<N, E, WEIGHT>& rhs
        ) const noexcept {
            return lhs.node < rhs.node;
        }

        bool operator()(
            const resident_slot_with_node<N, E, WEIGHT>& lhs,
            const N& rhs
        ) const noexcept {
            return lhs.node < rhs;
        }

        bool operator()(
            const N& lhs,
            const resident_slot_with_node<N, E, WEIGHT>& rhs
        ) const noexcept {
            return lhs < rhs.node;
        }
    };





    template<
        typename T,
        typename G
    >
    concept resident_slot_container_container_creator_pack =
        readable_sliceable_pairwise_alignment_graph<G>
        && container_creator_of_type<
            typename T::SLOT_CONTAINER_CREATOR,
            resident_slot_with_node<
                typename G::N,
                typename G::E,
                typename G::ED
            >
        >;

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G
    >
    struct resident_slot_container_heap_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        using SLOT_CONTAINER_CREATOR=vector_container_creator<resident_slot_with_node<N, E, ED>, debug_mode>;
    };

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt
    >
    struct resident_slot_container_stack_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        using SLOT_CONTAINER_CREATOR=static_vector_container_creator<
            resident_slot_with_node<N, E, ED>,
            G::limits(grid_down_cnt, grid_right_cnt).max_resident_nodes_cnt,
            debug_mode
        >;
    };




    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        resident_slot_container_container_creator_pack<G> CONTAINER_CREATOR_PACK=resident_slot_container_heap_container_creator_pack<debug_mode, G>
    >
    class resident_slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        using SLOT_CONTAINER_CREATOR=typename CONTAINER_CREATOR_PACK::SLOT_CONTAINER_CREATOR;
        using SLOT_CONTAINER=decltype(std::declval<SLOT_CONTAINER_CREATOR>().create_objects(0zu));

        SLOT_CONTAINER slots;

    public:
        resident_slot_container(
            const G& graph_
        )
        : slots{
            SLOT_CONTAINER_CREATOR {}.create_copy(
                graph_.resident_nodes()
                | std::views::transform([](const N& node) {
                    return resident_slot_with_node<N, E, ED> {
                        node,
                        {}
                    };
                })
            )
        } {
            std::ranges::sort(slots.begin(), slots.end(), resident_slot_with_node_comparator<N, E, ED>{});
        }

        std::optional<std::reference_wrapper<resident_slot<E, ED>>> find(const N& node) {
            auto it {
                std::lower_bound(
                    slots.begin(),
                    slots.end(),
                    node,
                    resident_slot_with_node_comparator<N, E, ED>{}
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
