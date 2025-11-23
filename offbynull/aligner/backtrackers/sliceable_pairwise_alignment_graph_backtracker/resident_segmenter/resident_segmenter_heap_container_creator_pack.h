#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_RESIDENT_SEGMENTER_HEAP_CONTAINER_CREATOR_PACK_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_RESIDENT_SEGMENTER_HEAP_CONTAINER_CREATOR_PACK_H

#include <cstddef>
#include <vector>
#include <variant>
#include <ranges>
#include <type_traits>
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_node.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtrackable_edge.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter/hop.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter/segment.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter/unimplemented_resident_segmenter_container_creator_pack.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker/bidi_walker_heap_container_creator_pack.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter
        ::resident_segmenter_heap_container_creator_pack {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_node::backtrackable_node;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::hop::hop;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::segment::segment;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::bidi_walker
        ::bidi_walker_heap_container_creator_pack::bidi_walker_heap_container_creator_pack;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::range_of_type;

    /**
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::resident_segmenter_container_creator_pack::resident_segmenter_container_creator_pack
     * that allocates its containers on the heap.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam N Graph node type.
     * @tparam E Graph edge type.
     * @tparam ED Graph edge data type (edge weight).
     * @tparam minimize_allocations `true` to force created containers to prime with the exact number of elements needed, thereby
     *     removing/reducing the need for adhoc reallocations.
     */
    template<
        bool debug_mode,
        backtrackable_node N,
        backtrackable_edge E,
        weight ED,
        bool minimize_allocations
    >
    struct resident_segmenter_heap_container_creator_pack {
        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::unimplemented_resident_segmenter_container_creator_pack::unimplemented_resident_segmenter_container_creator_pack::create_bidi_walker_container_creator_pack
         */
        bidi_walker_heap_container_creator_pack<
            debug_mode,
            N,
            E,
            ED,
            minimize_allocations
        > create_bidi_walker_container_creator_pack() const {
            return {};
        }

        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::unimplemented_resident_segmenter_container_creator_pack::unimplemented_resident_segmenter_container_creator_pack::create_resident_node_container
         */
        std::vector<N> create_resident_node_container(range_of_type<N> auto&& resident_nodes) const {
            if constexpr (std::ranges::sized_range<std::remove_cvref_t<decltype(resident_nodes)>>) {
                std::vector<N> ret {};
                ret.reserve(std::ranges::size(resident_nodes));
                for (const auto& n : resident_nodes) {
                    ret.push_back(n);
                }
                return ret;
            } else if constexpr (minimize_allocations) {
                // Is this a bad idea? Calling std::ranges::distance() on large input much slower vs vector heap resizing that happens?
                std::vector<N> ret {};
                ret.reserve(std::ranges::distance(resident_nodes.begin(), resident_nodes.end()));
                for (const auto& n : resident_nodes) {
                    ret.push_back(n);
                }
                return ret;
            } else {
                return std::vector<N>(resident_nodes.begin(), resident_nodes.end());
            }
        }

        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::unimplemented_resident_segmenter_container_creator_pack::unimplemented_resident_segmenter_container_creator_pack::create_resident_edge_container
         */
        std::vector<E> create_resident_edge_container(std::size_t resident_nodes_capacity) const {
            std::vector<E> ret {};
            if constexpr (minimize_allocations) {
                ret.reserve(resident_nodes_capacity);
            }
            return ret;
        }

        /**
         * @copydoc offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::resident_segmenter::unimplemented_resident_segmenter_container_creator_pack::unimplemented_resident_segmenter_container_creator_pack::create_segment_hop_chain_container
         */
        std::vector<
            std::variant<
                hop<E>,
                segment<N>
            >
        > create_segment_hop_chain_container(std::size_t resident_nodes_capacity) const {
            std::vector<std::variant<hop<E>, segment<N>>> ret {};
            if constexpr (minimize_allocations) {
                ret.reserve(resident_nodes_capacity * 2zu + 1zu);
            }
            return ret;
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_RESIDENT_SEGMENTER_RESIDENT_SEGMENTER_HEAP_CONTAINER_CREATOR_PACK_H
