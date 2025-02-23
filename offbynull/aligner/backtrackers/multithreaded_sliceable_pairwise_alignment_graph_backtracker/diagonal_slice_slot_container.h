#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_MULTITHREADED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_DIAGONAL_SLICE_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_MULTITHREADED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_DIAGONAL_SLICE_SLOT_CONTAINER_H

#include <functional>
#include <optional>
#include <utility>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <shared_mutex>
#include <mutex>
#include "offbynull/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/backtrackers/multithreaded_sliceable_pairwise_alignment_graph_backtracker/slot.h"
#include "offbynull/aligner/backtrackers/multithreaded_sliceable_pairwise_alignment_graph_backtracker/concepts.h"

namespace offbynull::aligner::backtrackers::multithreaded_sliceable_pairwise_alignment_graph_backtracker::diagonal_slice_slot_container {
    using offbynull::aligner::backtrackers::multithreaded_sliceable_pairwise_alignment_graph_backtracker::slot::slot;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::aligner::backtrackers::multithreaded_sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_node;
    using offbynull::aligner::backtrackers::multithreaded_sliceable_pairwise_alignment_graph_backtracker::concepts::backtrackable_edge;
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::concepts::unqualified_value_type;
    using offbynull::utils::static_vector_typer;


/*
1 2 3
2 3 4
3 4 5
4 5 6
5 6 7

1
2 2
3 3 3
4 4 4
5 5 5
6 6
7

1 2 3 4 5
2 3 4 5 6
3 4 5 6 7

1
2 2
3 3 3
4 4 4
5 5 5
6 6
7
*/


    template<
        typename T,
        typename E,
        typename ED
    >
    concept diagonal_slice_slot_container_container_creator_pack =
        unqualified_value_type<T>
        && backtrackable_edge<E>
        && weight<ED>
        && requires(const T t, std::size_t segment_cnt, std::size_t items_per_segment, std::size_t grid_depth_cnt) {
            { t.create_slot_container(segment_cnt, items_per_segment, grid_depth_cnt) } -> random_access_range_of_type<slot<E, ED>>;
            { t.create_mutex_container(segment_cnt) } -> random_access_range_of_type<std::shared_mutex>;
        };

    template<
        bool debug_mode,
        backtrackable_edge E,
        weight ED
    >
    struct diagonal_slice_slot_container_heap_container_creator_pack {
        std::vector<slot<E, ED>> create_slot_container(
            std::size_t segment_cnt,
            std::size_t items_per_segment,
            std::size_t grid_depth_cnt
        ) const {
            std::size_t cnt { items_per_segment * segment_cnt * grid_depth_cnt };
            return std::vector<slot<E, ED>>(cnt);
        }

        std::vector<std::shared_mutex> create_mutex_container(std::size_t segment_cnt) const {
            return std::vector<std::shared_mutex>(segment_cnt);
        }
    };

    template<
        bool debug_mode,
        backtrackable_edge E,
        weight ED,
        std::size_t segment_cnt,
        std::size_t items_per_segment,
        std::size_t grid_depth_cnt
    >
    struct diagonal_slice_slot_container_stack_container_creator_pack {
        static constexpr std::size_t max_elem_cnt { items_per_segment * segment_cnt * grid_depth_cnt };
        using SEGMENT_CONTAINER_TYPE = typename static_vector_typer<
            debug_mode,
            slot<E, ED>,
            max_elem_cnt
        >::type;
        SEGMENT_CONTAINER_TYPE create_slot_container(
            std::size_t segment_cnt_,
            std::size_t items_per_segment_,
            std::size_t grid_depth_cnt_
        ) const {
            std::size_t cnt { items_per_segment_ * segment_cnt_ * grid_depth_cnt_ };
            if constexpr (debug_mode) {
                if (cnt > max_elem_cnt) {
                    throw std::runtime_error { "Bad element count" };
                }
            }
            return SEGMENT_CONTAINER_TYPE(cnt);
        }

        using MUTEX_CONTAINER_TYPE = typename static_vector_typer<
            debug_mode,
            std::shared_mutex,
            segment_cnt
        >::type;
        MUTEX_CONTAINER_TYPE create_mutex_container(std::size_t segment_cnt_) const {
            if constexpr (debug_mode) {
                if (segment_cnt_ > segment_cnt) {
                    throw std::runtime_error { "Bad element count" };
                }
            }
            return MUTEX_CONTAINER_TYPE(segment_cnt);
        }
    };




    enum class axis : std::uint8_t {
        DOWN_FROM_TOP_LEFT = 0,
        RIGHT_FROM_BOTTOM_LEFT = 1
    };



    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        diagonal_slice_slot_container_container_creator_pack<
            typename G::E,
            typename G::ED
        > CONTAINER_CREATOR_PACK = diagonal_slice_slot_container_heap_container_creator_pack<
            debug_mode,
            typename G::E,
            typename G::ED
        >
    >
    class diagonal_slice_slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;
        using ND = typename G::ND;
        using ED = typename G::ED;
        using INDEX = typename G::INDEX;
        using SLOT_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_slot_container(0zu, 0zu, 0zu));
        using MUTEX_CONTAINER = decltype(std::declval<CONTAINER_CREATOR_PACK>().create_mutex_container(0zu));

        const G& g;
        MUTEX_CONTAINER mutexs;
        SLOT_CONTAINER slots;
        axis axis_;
        INDEX axis_position;

    public:
        diagonal_slice_slot_container(
            const G& g_,
            std::size_t segment_cnt,
            std::size_t items_per_segment,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : g { g_ }
        , mutexs {
            container_creator_pack.create_mutex_container(
                segment_cnt
            )
        }
        , slots {
            container_creator_pack.create_slot_container(
                segment_cnt,
                items_per_segment,
                g.grid_depth_cnt
            )
        }
        , axis_ { axis::DOWN_FROM_TOP_LEFT }
        , axis_position {} {}


FIND SHOULD TAKE IN WHICH WORKER IS ACCESSING, SO IT CAN RETURN LOCK (IF NESECCARY)
* LOCK SHOULD ONLY BE RETURNED IF ACCESSING last item IN previous SEGMENT (in relation to worker_segment_idx)
* LOCK SHOULD ONLY BE RETURNED IF ACCESSING last item IN current SEGMENT (in relation to worker_segment_idx)
* EVERYTHING ELSE BEING ACCESSED SHOULD BE non-last item IN current SEGMENT (in relation to worker_segment_idx)
the above is wrong? beginning diagonal slices wont fill each segment entirely ... imagine the following segments ...
   a a a _ _   b b b _ _   c c _ _ _ _
the last item in each segment is non-existant -- as such, its best to lock all access on the segment

        std::optional<std::pair<std::mutex, std::reference_wrapper<slot<E, ED>>>> find(const N& node) {
            TODO: FIX TO USE NEW PARAMS AND NEW RETURN TYPE!!
            TODO: FIX TO USE NEW PARAMS AND NEW RETURN TYPE!!
            TODO: FIX TO USE NEW PARAMS AND NEW RETURN TYPE!!
            /*
                              1
                              2 2
             1 2 3 4 5        3 3 3
             2 3 4 5 6        4 4 4
             3 4 5 6 7        5 5 5
                              6 6
                              7
             */
            const auto& [down_offset, right_offset, depth] { g.node_to_grid_offset(node) };
            switch (axis_) {
                case axis::DOWN_FROM_TOP_LEFT: {
                    if (down_offset > axis_position) {
                        return { std::nullopt };
                    }
                    INDEX expected_right_offset { axis_position - down_offset };
                    if (right_offset == expected_right_offset) {
                        return { { slots[expected_right_offset] } };
                    }
                    return { std::nullopt };
                }
                case axis::RIGHT_FROM_BOTTOM_LEFT: {
                    if (right_offset < axis_position) {
                        return { std::nullopt };
                    }
                    INDEX expected_down_offset { g.grid_down_cnt - (right_offset - axis_position) - 1 };
                    if (down_offset == expected_down_offset) {
                        return { { slots[expected_down_offset] } };
                    }
                    return { std::nullopt };
                }
                [[unlikely]] default: {
                    if constexpr (debug_mode) {
                        throw std::runtime_error { "This should never happen" };
                    }
                }
            }
            std::unreachable();
        }

        void reset(axis axis__, INDEX axis_position_) {
            this->axis_ = axis__;
            this->axis_position = axis_position_;
            if constexpr (debug_mode) {
                switch(axis__) {
                    case axis::DOWN_FROM_TOP_LEFT: {
                        if (axis_position_ >= g.grid_down_cnt) {
                            throw std::runtime_error { "Axis position too far down" };
                        }
                        break;
                    }
                    case axis::RIGHT_FROM_BOTTOM_LEFT: {
                        if (axis_position_ >= g.grid_right_cnt) {
                            throw std::runtime_error { "Axis position too far right" };
                        }
                        break;
                    }
                    [[unlikely]] default: {
                        throw std::runtime_error { "This should never happen" };
                    }
                }
            }
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_MULTITHREADED_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_DIAGONAL_SLICE_SLOT_CONTAINER_H
