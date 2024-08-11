#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_H

#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::container_creator_of_type;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::helpers::container_creators::static_vector_container_creator;

    template<typename E>
    struct element {
        element<E>* prev;
        element<E>* next;
        E backtracking_edge;

        element()
        : prev{}
        , next{}
        , backtracking_edge{} {}

        element(
            element<E>* prev_,
            element<E>* next_,
            E backtracking_edge_
        )
        : prev{prev_}
        , next{next_}
        , backtracking_edge{backtracking_edge_} {}

        bool operator==(const element &) const = default;
    };

    template<readable_sliceable_pairwise_alignment_graph G>
    class backward_walker_iterator {
    private:
        using N = typename G::N;
        using E = typename G::E;

        element<E>* current;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = E;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        backward_walker_iterator()
        : current {} {}

        backward_walker_iterator(element<E>* tail)
        : current { tail } {}

        reference operator*() const {
            return current->backtracking_edge;
        }

        backward_walker_iterator& operator++() {
            current = current->prev;
            return *this;
        }

        backward_walker_iterator operator++(int) {
            backward_walker_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const backward_walker_iterator& other) const {
            return current == other.current;
        }
    };

    template<readable_sliceable_pairwise_alignment_graph G>
    struct backward_walker_range {
    private:
        using E = typename G::E;

        element<E>* head;
        element<E>* tail;

    public:
        backward_walker_range(
            element<E>* head_,
            element<E>* tail_
        )
        : head{ head_ }
        , tail{ tail_ } {}

        backward_walker_iterator<G> begin() const {
            return backward_walker_iterator<G>{ tail };
        }

        backward_walker_iterator<G> end() const {
            return backward_walker_iterator<G>{ nullptr };
        }
    };









    template<
        typename T,
        typename G
    >
    concept path_container_container_creator_pack =
        readable_sliceable_pairwise_alignment_graph<G>
        && container_creator_of_type<typename T::ELEMENT_CONTAINER_CREATOR, element<typename G::E>>;

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G
    >
    struct path_container_heap_container_creator_pack {
        using E = typename G::E;
        using ED = typename G::ED;
        using ELEMENT_CONTAINER_CREATOR=vector_container_creator<element<typename G::E>, debug_mode>;
    };

    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        std::size_t grid_down_cnt,
        std::size_t grid_right_cnt
    >
    struct path_container_stack_container_creator_pack {
        using N = typename G::N;
        using E = typename G::E;
        using ED = typename G::ED;
        using ELEMENT_CONTAINER_CREATOR=static_vector_container_creator<
            element<typename G::E>,
            G::limits(grid_down_cnt, grid_right_cnt).max_path_edge_cnt,
            debug_mode
        >;
    };








    template<
        bool debug_mode,
        readable_sliceable_pairwise_alignment_graph G,
        path_container_container_creator_pack<G> CONTAINER_CREATOR_PACK=path_container_heap_container_creator_pack<debug_mode, G>
    >
    class path_container {
    private:
        using E = typename G::E;

        using ELEMENT_CONTAINER_CREATOR=typename CONTAINER_CREATOR_PACK::ELEMENT_CONTAINER_CREATOR;
        using ELEMENT_CONTAINER=decltype(std::declval<ELEMENT_CONTAINER_CREATOR>().create_empty(0zu));

        ELEMENT_CONTAINER element_container;
        element<E>* head;
        element<E>* tail;
        std::size_t next_idx;

    public:
        path_container(const G& g)
        : element_container{
            ELEMENT_CONTAINER_CREATOR {}.create_objects(
                G::limits(
                   g.grid_down_cnt,
                   g.grid_right_cnt
               ).max_path_edge_cnt
            )
        }
        , head{nullptr}
        , tail{nullptr}
        , next_idx{0zu} {}

        element<E>* initialize(const E& backtracking_edge) {
            if constexpr (debug_mode) {
                if (next_idx != 0zu) {
                    throw std::runtime_error("Already initialized");
                }
                if (next_idx >= element_container.size()) {
                    // If this happens, G::limits().max_path_edge_cnt is probably giving back a number that's too low
                    throw std::runtime_error("Container too small");
                }
            }
            element<E>* entry { &element_container[0zu] };
            entry->backtracking_edge = backtracking_edge;
            ++next_idx;
            head = entry;
            tail = entry;
            return entry;
        }

        element<E>* push_prefix(element<E>* entry, const E& backtracking_edge) {
            if constexpr (debug_mode) {
                if (next_idx == 0zu) {
                    throw std::runtime_error("Not initialized");
                }
                if (next_idx >= element_container.size()) {
                    // If this happens, G::limits().max_path_edge_cnt is probably giving back a number that's too low
                    throw std::runtime_error("Container too small");
                }
            }
            element<E>* prefix_entry { &element_container[next_idx] };
            prefix_entry->backtracking_edge = backtracking_edge;
            ++next_idx;
            element<E>* orig_prev { entry->prev };
            if (orig_prev != nullptr) {
                orig_prev->next = prefix_entry;
            }
            prefix_entry->prev = orig_prev;
            entry->prev = prefix_entry;
            prefix_entry->next = entry;
            if (entry == head) {
                head = prefix_entry;
            }
            return prefix_entry;
        }

        element<E>* push_suffix(element<E>* entry, const E& backtracking_edge) {
            if constexpr (debug_mode) {
                if (next_idx == 0zu) {
                    throw std::runtime_error("Not initialized");
                }
                if (next_idx >= element_container.size()) {
                    // If this happens, G::limits().max_path_edge_cnt is probably giving back a number that's too low
                    throw std::runtime_error("Container too small");
                }
            }
            element<E>* suffix_entry { &element_container[next_idx] };
            suffix_entry->backtracking_edge = backtracking_edge;
            ++next_idx;
            element<E>* orig_next { entry->next };
            if (orig_next != nullptr) {
                orig_next->prev = suffix_entry;
            }
            suffix_entry->next = orig_next;
            entry->next = suffix_entry;
            suffix_entry->prev = entry;
            if (entry == tail) {
                tail = suffix_entry;
            }
            return suffix_entry;
        }

        std::ranges::forward_range auto walk_path_backward() {
            return backward_walker_range<G> { head, tail };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_H
