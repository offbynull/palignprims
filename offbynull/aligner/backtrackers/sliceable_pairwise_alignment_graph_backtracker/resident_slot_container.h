#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_H

#include <functional>
#include <ranges>
#include <algorithm>
#include <vector>
#include <utility>
#include <optional>
#include <cstddef>
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/slot.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_slot_container {
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::slot::slot;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::range_of_type;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::concepts::unqualified_value_type;
    using offbynull::utils::static_vector_typer;

    template<
        typename E,
        weight WEIGHT
    >
    struct resident_slot {
        bool initialized;
        slot<E, WEIGHT> slot_;
    };

    template<
        typename N,
        typename E,
        weight WEIGHT
    >
    struct resident_slot_with_node {
        N node;
        resident_slot<E, WEIGHT> slot_;
    };

    template<
        typename N,
        typename E,
        weight WEIGHT
    >
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
        typename N,
        typename E,
        typename ED
    >
    concept resident_slot_container_container_creator_pack =
        unqualified_value_type<T>
        && weight<ED>
        && requires(
            const T t,
            const std::vector<resident_slot_with_node<N, E, ED>>& fake_range
        ) {
            { t.create_slot_container(fake_range) } -> random_access_range_of_type<resident_slot_with_node<N, E, ED>>;
        };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight ED
    >
    struct resident_slot_container_heap_container_creator_pack {
        std::vector<resident_slot_with_node<N, E, ED>> create_slot_container(
            range_of_type<resident_slot_with_node<N, E, ED>> auto&& r
        ) const {
            return std::vector<resident_slot_with_node<N, E, ED>>(r.begin(), r.end());
        }
    };

    template<
        bool debug_mode,
        typename N,
        typename E,
        weight ED,
        std::size_t resident_nodes_capacity
    >
    struct resident_slot_container_stack_container_creator_pack {
        using CONTAINER_TYPE = typename static_vector_typer<
            debug_mode,
            resident_slot_with_node<N, E, ED>,
            resident_nodes_capacity
        >::type;
        CONTAINER_TYPE create_slot_container(range_of_type<resident_slot_with_node<N, E, ED>> auto&& r) const  {
            return CONTAINER_TYPE(r.begin(), r.end());
        }
    };




    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        resident_slot_container_container_creator_pack<
            typename G::N,
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = resident_slot_container_heap_container_creator_pack<
            debug_mode,
            typename G::N,
            typename G::E,
            typename G::ED
        >
    >
    class resident_slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        using SLOT_CONTAINER = decltype(
            std::declval<CONTAINER_CREATOR_PACK>().create_slot_container(
                std::declval<std::vector<resident_slot_with_node<N, E, ED>>>()
            )
        );

        SLOT_CONTAINER slots;

    public:
        resident_slot_container(
            const G& g,
            CONTAINER_CREATOR_PACK container_creator_pack= {}
        )
        : slots {
            container_creator_pack.create_slot_container(
                g.resident_nodes()
                | std::views::transform([](const N& node) {
                    return resident_slot_with_node<N, E, ED> {
                        node,
                        {}
                    };
                })
            )
        } {
            std::ranges::sort(slots.begin(), slots.end(), resident_slot_with_node_comparator<N, E, ED> {});
        }

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

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SLOT_CONTAINER_H
