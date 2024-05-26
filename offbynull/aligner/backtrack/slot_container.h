#ifndef OFFBYNULL_ALIGNER_BACKTRACK_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACK_SLOT_CONTAINER_H

#include <cstddef>
#include <iterator>
#include "offbynull/aligner/backtrack/container_creator.h"
#include "offbynull/aligner/backtrack/container_creators.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrack::slot_container {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::backtrack::container_creator::container_creator;
    using offbynull::aligner::backtrack::container_creators::vector_container_creator;
    using offbynull::concepts::input_iterator_of_type;
    using offbynull::concepts::widenable_to_size_t;

    template<typename N, typename E, widenable_to_size_t COUNT, weight WEIGHT>
    struct slot {
        N node;
        COUNT unwalked_parent_cnt;
        E backtracking_edge;
        WEIGHT backtracking_weight;

        slot(N node_, COUNT unwalked_parent_cnt_)
        : node{node_}
        , unwalked_parent_cnt{unwalked_parent_cnt_}
        , backtracking_edge{}
        , backtracking_weight{} {}


        slot()
        : node{}
        , unwalked_parent_cnt{}
        , backtracking_edge{}
        , backtracking_weight{} {}
    };

    template<
        typename N,
        typename E,
        widenable_to_size_t INDEX,
        weight WEIGHT,
        container_creator ALLOCATOR=vector_container_creator<slot<N, E, INDEX, WEIGHT>>,
        bool error_check=true
    >
    class slot_container {
    private:
        struct slots_comparator {
            bool operator()(const slot<N, E, INDEX, WEIGHT>& lhs, const slot<N, E, INDEX, WEIGHT>& rhs) const noexcept {
                return lhs.node < rhs.node;
            }

            bool operator()(const slot<N, E, INDEX, WEIGHT>& lhs, const N& rhs) const noexcept {
                return lhs.node < rhs;
            }

            bool operator()(const N& lhs, const slot<N, E, INDEX, WEIGHT>& rhs) const noexcept {
                return lhs < rhs.node;
            }
        };

        decltype(std::declval<ALLOCATOR>().create_empty(std::nullopt)) slots;
    public:
        slot_container(
            input_iterator_of_type<slot<N, E, INDEX, WEIGHT>> auto&& begin,
            input_iterator_of_type<slot<N, E, INDEX, WEIGHT>> auto&& end,
            ALLOCATOR container_creator = {}
        ) : slots(container_creator.create_copy(begin, end)) {
            std::ranges::sort(
                slots.begin(),
                slots.end(),
                slots_comparator{}
            );
        }

        std::size_t find_idx(const N& node){
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator{}) };
            return it - slots.begin();
        }

        slot<N, E, INDEX, WEIGHT>& find_ref(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator{}) };
            return *it;
        }

        slot<N, E, INDEX, WEIGHT>& at_idx(const std::size_t idx) {
            return slots[idx];
        }

        std::pair<std::size_t, slot<N, E, INDEX, WEIGHT>&> find(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator{}) };
            auto dist_from_beginning { std::ranges::distance(slots.begin(), it) };
            std::size_t idx;
            if constexpr (error_check && !widenable_to_size_t<decltype(dist_from_beginning)>) {
                // It may be that dist_from_beginning is signed, in which case the widenable_to_size_t fails (it tests
                // for unsignedness in addition testing to see if widenable). If it is signed, check the max value of
                // both types. If max_value(decltype(dist_from_beginning)) < max_value(size_t), it's safe to do a
                // static_cast to size_t because dist_from_beginning should never be a negative value.
                if constexpr (std::numeric_limits<decltype(dist_from_beginning)>::max() > std::numeric_limits<std::size_t>::max()) {
                    if (dist_from_beginning > std::numeric_limits<std::size_t>::max()) {
                        throw std::runtime_error("Narrowed!");
                    }
                }
                idx = { static_cast<std::size_t>(dist_from_beginning) };
            } else {
                idx = { dist_from_beginning };
            }
            slot<N, E, INDEX, WEIGHT>& slot { *it };
            return { idx, slot };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_SLOT_CONTAINER_H
