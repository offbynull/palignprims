#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_FORWARD_WALKER_RANGE_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_FORWARD_WALKER_RANGE_H

#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/path_container/element.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/path_container/forward_walker_iterator.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::forward_walker_range {
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::backtrackable_edge::backtrackable_edge;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::element::element;
    using offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::forward_walker_iterator
        ::forward_walker_iterator;

    /**
     * Range exposing the edges of an
     * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::path_container::path_container
     * in reverse.
     *
     * @tparam E Graph edge identifier type.
     */
    template<backtrackable_edge E>
    struct forward_walker_range {
    private:
        element<E>* head;
        element<E>* tail;

    public:
        /**
         * Construct an
         * @ref offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container::forward_walker_range::forward_walker_range.
         *
         * @param head_ First element (edge) in the path container.
         * @param tail_ Last element (edge) in the same path container as `head_`.
         */
        forward_walker_range(
            element<E>* head_,
            element<E>* tail_
        )
        : head { head_ }
        , tail { tail_ } {}

        // No point in documenting anything below here. These are the barebones functions required by a range?

        forward_walker_iterator<E> begin() const {
            return forward_walker_iterator<E> { head };
        }

        forward_walker_iterator<E> end() const {
            return forward_walker_iterator<E> { nullptr };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_FORWARD_WALKER_RANGE_H
