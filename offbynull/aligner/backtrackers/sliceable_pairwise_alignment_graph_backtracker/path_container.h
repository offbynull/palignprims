#ifndef OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_H
#define OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_H

#include "offbynull/helpers/container_creators.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_walker.h"

namespace offbynull::aligner::backtrackers::sliceable_pairwise_alignment_graph_backtracker::path_container {
    using offbynull::aligner::concepts::weight;
    using offbynull::helpers::container_creators::container_creator;
    using offbynull::helpers::container_creators::vector_container_creator;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;

    template<typename E>
    struct element {
        element<E>* prev;
        element<E>* next;
        E backtracking_edge;

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
        G* g;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = E;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        backward_walker_iterator()
        : g {}
        , current {} {}

        backward_walker_iterator(G& g_, element<E>* tail)
        : g { &g_ }
        , current { tail } {}

        reference operator*() const {
            return current->backtracking_edge;
        }

        backward_walker_iterator& operator++() {
            const N& next_edge_src { g->get_edge_from(current->backtracking_edge) };
            while (true) {
                current = current->prev;
                const N& prev_edge_dst { g->get_edge_to(current->backtracking_edge) };
                if (next_edge_src == prev_edge_dst) {
                    break;
                }
                if (current == nullptr) {
                    break;
                }
            }
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

        G& g;
        element<E>* head;
        element<E>* tail;

    public:
        backward_walker_range(
            G& g_,
            element<E>* head_,
            element<E>* tail_
        )
        : g{ g_ }
        , head{ head_ }
        , tail{ tail_ } {}

        backward_walker_iterator<G> begin() {
            return backward_walker_iterator<G>{ g, tail };
        }

        backward_walker_iterator<G> end() {
            return backward_walker_iterator<G>{ g, nullptr };
        }
    };

    template<
        readable_sliceable_pairwise_alignment_graph G,
        container_creator ELEMENT_CONTAINER_CREATOR=vector_container_creator<element<typename G::E>>,
        bool error_check=true
    >
    class path_container {
    private:
        using E = typename G::E;
        using ELEMENT_CONTAINER=decltype(std::declval<ELEMENT_CONTAINER_CREATOR>().create_empty(0zu));

    public:
        ELEMENT_CONTAINER element_container;
        element<E>* head;
        element<E>* tail;

        path_container(
            std::size_t max_path_edge_cnt,
            ELEMENT_CONTAINER_CREATOR element_container_creator = {}
        )
        : element_container{ element_container_creator.create_empty(max_path_edge_cnt) } {}

        element<E>* initialize(const E& backtracking_edge) {
            if constexpr (error_check) {
                if (!element_container.empty()) {
                    throw std::runtime_error("Already initialized");
                }
            }
            element_container.push_back(
                element<E> {
                    nullptr,
                    nullptr,
                    backtracking_edge
                }
            );
            element<E>* entry { &element_container[0zu] };
            head = entry;
            tail = entry;
            return entry;
        }

        element<E>* push_prefix(element<E>* entry, const E& backtracking_edge) {
            if constexpr (error_check) {
                if (element_container.empty()) {
                    throw std::runtime_error("Not initialized");
                }
            }
            element_container.push_back(
                element<E> {
                    nullptr,
                    nullptr,
                    backtracking_edge
                }
            );
            element<E>* prefix_entry { &element_container[element_container.size() - 1zu] };
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
            if constexpr (error_check) {
                if (element_container.empty()) {
                    throw std::runtime_error("Not initialized");
                }
            }
            element_container.push_back(
                element<E> {
                    nullptr,
                    nullptr,
                    backtracking_edge
                }
            );
            element<E>* suffix_entry { &element_container[element_container.size() - 1zu] };
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

        std::ranges::forward_range auto walk_path_backward(G& g) {
            return backward_walker_range<G> { g, head, tail };
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_H
