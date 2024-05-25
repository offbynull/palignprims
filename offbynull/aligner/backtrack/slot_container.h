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

    template<typename N, typename E, std::unsigned_integral INDEXER, weight WEIGHT>
    struct slot {
        N node;
        INDEXER unwalked_parent_cnt;
        E backtracking_edge;
        WEIGHT backtracking_weight;

        slot(N node_, INDEXER unwalked_parent_cnt_)
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
        std::unsigned_integral INDEXER,
        weight WEIGHT,
        container_creator ALLOCATOR=vector_container_creator<slot<N, E, INDEXER, WEIGHT>>
    >
    class slot_container {
    private:
        struct slots_comparator {
            bool operator()(const slot<N, E, INDEXER, WEIGHT>& lhs, const slot<N, E, INDEXER, WEIGHT>& rhs) const noexcept {
                return lhs.node < rhs.node;
            }

            bool operator()(const slot<N, E, INDEXER, WEIGHT>& lhs, const N& rhs) const noexcept {
                return lhs.node < rhs;
            }

            bool operator()(const N& lhs, const slot<N, E, INDEXER, WEIGHT>& rhs) const noexcept {
                return lhs < rhs.node;
            }
        };

        decltype(std::declval<ALLOCATOR>().create_empty(std::nullopt)) slots;
    public:
        slot_container(
            input_iterator_of_type<slot<N, E, INDEXER, WEIGHT>> auto&& begin,
            input_iterator_of_type<slot<N, E, INDEXER, WEIGHT>> auto&& end,
            ALLOCATOR container_creator = {}
        ) : slots(container_creator.create_copy(begin, end)) {
            std::ranges::sort(
                slots.begin(),
                slots.end(),
                slots_comparator{}
            );
        }

        size_t find_idx(const N& node){
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator{}) };
            return it - slots.begin();
        }

        slot<N, E, INDEXER, WEIGHT>& find_ref(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator{}) };
            return *it;
        }

        slot<N, E, INDEXER, WEIGHT>& at_idx(const size_t idx) {
            return slots[idx];
        }

        std::pair<size_t, slot<N, E, INDEXER, WEIGHT>&> find(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator{}) };
            auto dist_from_beginning { std::ranges::distance(slots.begin(), it) };
            size_t idx { static_cast<size_t>(dist_from_beginning) };
            slot<N, E, INDEXER, WEIGHT>& slot { *it };
            return { idx, slot };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_SLOT_CONTAINER_H
