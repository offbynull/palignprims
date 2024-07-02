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

    template<typename E>
    class iterator {
    private:
        element<E>* current;
        element<E>* tail;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = element<E>;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

        iterator()
        : current{}
        , tail{} {}

        iterator(element<E>* head, element<E>* tail)
        : current{ head }
        , tail{ tail } {}

        reference operator*() const {
            return *current;
        }

        iterator& operator++() {
            current = current->next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator& operator--() {
            if (current == nullptr) {
                current = tail;
            } else {
                current = current->prev;
            }
            return *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return current == other.current;
        }
    };

    template<typename E>
    struct internal_range {
    private:
        element<E>* head;
        element<E>* tail;

    public:
        internal_range(
            element<E>* head_,
            element<E>* tail_
        )
        : head{ head_ }
        , tail{ tail_} {}

        iterator<E> begin() {
            return iterator<E>{ head, tail };
        }

        iterator<E> end() {
            return iterator<E>{ nullptr, tail };
        }
    };

    static_assert(std::ranges::bidirectional_range<internal_range<int>>);
    static_assert(std::ranges::viewable_range<internal_range<int>>);

    template<
        typename N,
        typename E,
        container_creator ELEMENT_CONTAINER_CREATOR=vector_container_creator<element<E>>,
        bool error_check=true
    >
    class path_container {
    private:
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

        internal_range<E> to_range() {
            return { head, tail };
        }

        template<readable_sliceable_pairwise_alignment_graph GRAPH>
        std::ranges::range auto walk_path_backward(GRAPH& g) {
            N sink_node { g.get_edge_to(tail->backtracking_edge) };
            return
                internal_range{ head, tail }
                | std::views::reverse
                | std::views::transform([current = tail](const auto&) mutable {
                    element<E>* ret { current };
                    current = ret->prev;
                    return ret;
                })
                | std::views::take_while([](element<E>* e) { return e != nullptr; })
                | std::views::transform([&g, expected_dst = sink_node](element<E>* e) mutable {
                    if (g.get_edge_to(e->backtracking_edge) == expected_dst) {
                        expected_dst = g.get_edge_to(e->backtracking_edge);
                        return std::optional<E> { e->backtracking_edge };
                    } else {
                        return std::optional<E> { std::nullopt };
                    }
                })
                | std::views::filter([](const std::optional<E>& backtracking_edge) {
                    return backtracking_edge.has_value();
                })
                | std::views::transform([](const std::optional<E>& backtracking_edge) {
                    return *backtracking_edge;
                });
        }
    };
}

#endif //OFFBYNULL_ALIGNER_BACKTRACKERS_SLICEABLE_PAIRWISE_ALIGNMENT_GRAPH_BACKTRACKER_PATH_CONTAINER_H
