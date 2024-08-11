#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_H

#include <cstddef>
#include <ranges>
#include <boost/container/small_vector.hpp>
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::backtrackers::graph_backtracker::slot_container {
    using offbynull::aligner::concepts::weight;
    using offbynull::concepts::input_iterator_of_type;
    using offbynull::concepts::widenable_to_size_t;
    using offbynull::concepts::random_access_range_of_type;
    using offbynull::aligner::graph::graph::readable_graph;

    template<typename N, typename E, weight WEIGHT>
    struct slot {
        N node;
        std::size_t unwalked_parent_cnt;
        E backtracking_edge;
        WEIGHT backtracking_weight;

        slot(N node_, std::size_t unwalked_parent_cnt_)
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
        weight WEIGHT
    >
    struct slots_comparator {
        bool operator()(const slot<N, E, WEIGHT>& lhs, const slot<N, E, WEIGHT>& rhs) const noexcept {
            return lhs.node < rhs.node;
        }

        bool operator()(const slot<N, E, WEIGHT>& lhs, const N& rhs) const noexcept {
            return lhs.node < rhs;
        }

        bool operator()(const N& lhs, const slot<N, E, WEIGHT>& rhs) const noexcept {
            return lhs < rhs.node;
        }
    };





    template<
        typename T,
        typename G,
        typename WEIGHT
    >
    concept slot_container_container_creator_pack =
    readable_graph<G>
    && weight<WEIGHT>
    && requires(T t, std::vector<slot<typename G::N, typename G::E, WEIGHT>> fake_range) {
        { t.create_slot_container(fake_range.begin(), fake_range.end()) } -> random_access_range_of_type<slot<typename G::N, typename G::E, WEIGHT>>;
    };

    template<
        bool debug_mode,
        readable_graph G,
        weight WEIGHT
    >
    struct slot_container_heap_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;

        std::vector<slot<N, E, WEIGHT>> create_slot_container(auto begin, auto end) {
            return std::vector<slot<N, E, WEIGHT>>(begin, end);
        }
    };

    template<
        bool debug_mode,
        readable_graph G,
        weight WEIGHT,
        std::size_t heap_escape_size = 100zu
    >
    struct slot_container_stack_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;

        boost::container::small_vector<slot<N, E, WEIGHT>, heap_escape_size> create_slot_container(auto begin, auto end) {
            return boost::container::small_vector<slot<N, E, WEIGHT>, heap_escape_size>(begin, end);
        }
    };





    template<
        bool debug_mode,
        readable_graph G,
        weight WEIGHT,
        slot_container_container_creator_pack<G, WEIGHT> CONTAINER_CREATOR_PACK=slot_container_heap_container_creator_pack<debug_mode, G, WEIGHT>
    >
    class slot_container {
    private:
        using N = typename G::N;
        using E = typename G::E;

        using SLOT_CONTAINER=decltype(
            std::declval<CONTAINER_CREATOR_PACK>().create_slot_container(
                std::declval<std::vector<slot<N, E, WEIGHT>>>().begin(),
                std::declval<std::vector<slot<N, E, WEIGHT>>>().end()
            )
        );

        const G& g;
        SLOT_CONTAINER slots;
    public:
        // Concepts for params have been commented out because THEY FAIL when you pass in a std::views::common(...)'s
        // iterator. Apparently the iterator doesn't contain ::value_type? Or the iterator that's passed in isn't
        // default constructible (there doesn't seem to be any requirement that an iterator be default constructible,
        // but the iterator is a concat_view which is being wrapped in a std::views::common() and there's some weird
        // concepts checking to make sure things are default constructible / ::value_type isn't making it through?
        //
        // https://www.reddit.com/r/cpp_questions/comments/1d5z7sh/bizarre_requirement_of_stdinput_iterator/
        slot_container(
            const G& g_,
            /*input_iterator_of_type<slot<N, E, COUNT, WEIGHT>>*/ auto begin,
            /*std::sentinel_for<decltype(begin)>*/ auto end,
            CONTAINER_CREATOR_PACK container_creator_pack = {}
        )
        : g { g_ }
        , slots { container_creator_pack.create_slot_container(begin, end) } {
            std::ranges::sort(
                slots.begin(),
                slots.end(),
                slots_comparator<N, E, WEIGHT>{}
            );
        }

        std::size_t find_idx(const N& node){
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator<N, E, WEIGHT>{}) };
            return it - slots.begin();
        }

        slot<N, E, WEIGHT>& find_ref(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator<N, E, WEIGHT>{}) };
            return *it;
        }

        slot<N, E, WEIGHT>& at_idx(const std::size_t idx) {
            return slots[idx];
        }

        std::pair<std::size_t, slot<N, E, WEIGHT>&> find(const N& node) {
            auto it { std::lower_bound(slots.begin(), slots.end(), node, slots_comparator<N, E, WEIGHT>{}) };
            auto dist_from_beginning { std::ranges::distance(slots.begin(), it) };
            std::size_t idx;
            if constexpr (debug_mode && !widenable_to_size_t<decltype(dist_from_beginning)>) {
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
            slot<N, E, WEIGHT>& slot { *it };
            return { idx, slot };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_GRAPH_BACKTRACKER_SLOT_CONTAINER_H
