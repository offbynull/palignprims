#ifndef OFFBYNULL_ALIGNER_BACKTRACK_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACK_SLOT_CONTAINER_H

#include <cstddef>
#include <iterator>
#include "offbynull/aligner/backtrack/allocator.h"
#include "offbynull/aligner/backtrack/allocators.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrack::slot_container {
    using offbynull::aligner::backtrack::allocator::allocator;
    using offbynull::aligner::backtrack::allocators::VectorAllocator;
    using offbynull::concepts::input_iterator_of_type;

    template<typename N, typename E>
    struct slot {
        N node;
        size_t unwalked_parent_cnt;
        E backtracking_edge;
        double backtracking_weight;

        slot(N node_, size_t unwalked_parent_cnt_)
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
        allocator ALLOCATOR=VectorAllocator<slot<N, E>>
    >
    class slot_container {
    private:
        struct slots_comparator {
            bool operator()(const slot<N, E>& lhs, const slot<N, E>& rhs) const noexcept {
                return lhs.node < rhs.node;
            }

            bool operator()(const slot<N, E>& lhs, const N& rhs) const noexcept {
                return lhs.node < rhs;
            }

            bool operator()(const N& lhs, const slot<N, E>& rhs) const noexcept {
                return lhs < rhs.node;
            }
        };

        decltype(std::declval<ALLOCATOR>().allocate(0u)) slots;
    public:
        slot_container(
            input_iterator_of_type<slot<N, E>> auto&& begin,
            input_iterator_of_type<slot<N, E>> auto&& end,
            ALLOCATOR container_creator = {}
        ) : slots(container_creator.allocate(begin, end)) {
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

        slot<N, E>& find_ref(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator{}) };
            return *it;
        }

        slot<N, E>& at_idx(const size_t idx) {
            return slots[idx];
        }

        std::pair<size_t, slot<N, E>&> find(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator{}) };
            auto dist_from_beginning { std::ranges::distance(slots.begin(), it) };
            size_t idx { static_cast<size_t>(dist_from_beginning) };
            slot<N, E>& slot { *it };
            return { idx, slot };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_SLOT_CONTAINER_H
