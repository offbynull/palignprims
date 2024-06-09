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
        widenable_to_size_t COUNT,
        weight WEIGHT
    >
    struct slots_comparator {
        bool operator()(const slot<N, E, COUNT, WEIGHT>& lhs, const slot<N, E, COUNT, WEIGHT>& rhs) const noexcept {
            return lhs.node < rhs.node;
        }

        bool operator()(const slot<N, E, COUNT, WEIGHT>& lhs, const N& rhs) const noexcept {
            return lhs.node < rhs;
        }

        bool operator()(const N& lhs, const slot<N, E, COUNT, WEIGHT>& rhs) const noexcept {
            return lhs < rhs.node;
        }
    };

    template<
        typename N,
        typename E,
        widenable_to_size_t COUNT,
        weight WEIGHT,
        container_creator ALLOCATOR=vector_container_creator<slot<N, E, COUNT, WEIGHT>>,
        bool error_check=true
    >
    class slot_container {
    private:
        decltype(std::declval<ALLOCATOR>().create_empty(std::nullopt)) slots;
    public:
        // Concepts for params have been commented out because THEY FAIL when you pass in a std::views::common(...)'s
        // iterator. Apparently the iterator doesn't contain ::value_type? Or the iterator that's passed in isn't
        // default constructible (there doesn't seem to be any requirement that an iterator be default constructible,
        // but the iterator is a concat_view which is being wrapped in a std::views::common() and there's some weird
        // concepts checking to make sure things are default constructible / ::value_type isn't making it through?
        //
        // https://www.reddit.com/r/cpp_questions/comments/1d5z7sh/bizarre_requirement_of_stdinput_iterator/
        slot_container(
            /*input_iterator_of_type<slot<N, E, COUNT, WEIGHT>>*/ auto begin,
            /*std::sentinel_for<decltype(begin)>*/ auto end,
            ALLOCATOR container_creator = {}
        )
        : slots(container_creator.create_copy(begin, end)) {
            std::ranges::sort(
                slots.begin(),
                slots.end(),
                slots_comparator<N, E, COUNT, WEIGHT>{}
            );
        }

        std::size_t find_idx(const N& node){
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator<N, E, COUNT, WEIGHT>{}) };
            return it - slots.begin();
        }

        slot<N, E, COUNT, WEIGHT>& find_ref(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator<N, E, COUNT, WEIGHT>{}) };
            return *it;
        }

        slot<N, E, COUNT, WEIGHT>& at_idx(const std::size_t idx) {
            return slots[idx];
        }

        std::pair<std::size_t, slot<N, E, COUNT, WEIGHT>&> find(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator<N, E, COUNT, WEIGHT>{}) };
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
            slot<N, E, COUNT, WEIGHT>& slot { *it };
            return { idx, slot };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACK_SLOT_CONTAINER_H
