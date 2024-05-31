#ifndef OFFBYNULL_UTILS_H
#define OFFBYNULL_UTILS_H

#include <cstddef>
#include <functional>

#include "utils.h"
#include "boost/container/static_vector.hpp"
#include "boost/container/options.hpp"

namespace offbynull::utils {
    // https://www.reddit.com/r/cpp_questions/comments/1cp0rwu/how_to_lazily_concatenate_two_views/
    template <typename R1, typename R2>
    class concat_view : public std::ranges::view_interface<concat_view<R1, R2>> {
        R1 first_range;
        R2 second_range;

       public:
        template <typename T1, typename T2>
        concat_view(T1&& first, T2&& second)
            : first_range(std::forward<T1>(first)),
              second_range(std::forward<T2>(second)) {}

        concat_view(concat_view<R1, R2>& other) = default;
        concat_view(concat_view<R1, R2>&& other) = default;
        concat_view<R1, R2>& operator=(const concat_view<R1, R2>& other) = default;
        concat_view<R1, R2>& operator=(concat_view<R1, R2>&& other) = default;

        class sentinel {
        public:
            // need to provide sentinel == iterator separately
            bool operator==(const auto& i) const {
                return i == *this;
            }
        };

        template <typename It1, typename S1, typename It2, typename S2>
        requires std::same_as<decltype(*std::declval<It1>()), decltype(*std::declval<It2>())>
        class iterator {
            It1 it1;
            S1 end1;
            It2 it2;
            S2 end2;
            bool is_first;

            public:
            using difference_type = std::ptrdiff_t;
            using value_type = typename It1::value_type;
            // using reference = typename It1::reference;
            using iterator_category = std::input_iterator_tag;

            iterator(It1 a_first, S1 a_last, It2 b_first, S2 b_last)
            : it1(a_first)
            , end1(a_last)
            , it2(b_first)
            , end2(b_last)
            , is_first(a_first == a_last ? false : true) {}

            decltype(*it1) operator*() const {
                if (is_first) {
                    return *it1;
                } else {
                    return *it2;
                }
            }

            iterator& operator++() {
                if (is_first) {
                    if (++it1 == end1) {
                        is_first = false;
                    }
                } else {
                    ++it2;
                }
                return *this;
            }

            iterator operator++(int) {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const iterator& other) const {
                return is_first == other.is_first &&
                       (is_first ? it1 == other.it1 : it2 == other.it2);
            }

            bool operator==(const concat_view<R1, R2>::sentinel&) const {
                return !is_first && it2 == end2;
            }

        };

        auto begin() {
            return iterator(
                std::ranges::begin(first_range), std::ranges::end(first_range),
                std::ranges::begin(second_range), std::ranges::end(second_range)
            );
        }

        sentinel end() noexcept { return {}; }
    };

    // Proper placement of the deduction guide
    template <typename T1, typename T2>
    concat_view(T1&&, T2&&) -> concat_view<std::views::all_t<T1>, std::views::all_t<T2>>;

    struct concat_range_adaptor {
        template<typename R1, typename R2>
        constexpr auto operator() (R1&& r1, R2&& r2) const {
            return concat_view(std::forward<R1>(r1), std::forward<R2>(r2));
        }
    };

    template<typename R1, typename R2, typename Adaptor>
    auto operator|(const concat_view<R1, R2>& cv, Adaptor&& adaptor) {
        return adaptor(std::views::all(cv));
    }

    template<typename R1, typename R2, typename Adaptor>
    auto operator|(concat_view<R1, R2>&& cv, Adaptor&& adaptor) {
        return adaptor(std::views::all(std::move(cv)));
    }





    // Use this as opposed to std::views::cartesian_product(std::views::iota(...), std::views::iota(...)). See
    // https://www.reddit.com/r/cpp_questions/comments/1d2qecv/use_of_views_results_in_4x_the_number_of_assembly/.
    template <std::integral T>
    class pair_counter_view : public std::ranges::view_interface<pair_counter_view<T>> {
        T dim1_cnt;
        T dim2_cnt;

        public:
        pair_counter_view(T dim1_cnt_, T dim2_cnt_)
            : dim1_cnt(dim1_cnt_)
            , dim2_cnt(dim2_cnt_) {}

        pair_counter_view(pair_counter_view<T>& other) = default;
        pair_counter_view(pair_counter_view<T>&& other) = default;
        pair_counter_view<T>& operator=(const pair_counter_view<T>& other) = default;
        pair_counter_view<T>& operator=(pair_counter_view<T>&& other) = default;

        class sentinel {
        public:
            // need to provide sentinel == iterator separately
            bool operator==(const auto& i) const {
                return i == *this;
            }
        };

        class iterator {
            T dim1;
            T dim1_cnt;
            T dim2;
            T dim2_cnt;

           public:
            using difference_type = std::ptrdiff_t;
            using value_type = std::pair<T, T>;
            using pointer = value_type*;
            using reference = value_type;
            using iterator_category = std::input_iterator_tag;

            iterator(T dim1_, const T dim1_cnt_, T dim2_, const T dim2_cnt_)
            : dim1(dim1_)
            , dim1_cnt(dim1_cnt_)
            , dim2(dim2_)
            , dim2_cnt(dim2_cnt_) {}

            reference operator*() const {
                return std::pair<T, T> {dim1, dim2};
            }

            iterator& operator++() {
                dim1++;
                if (dim1 == dim1_cnt) {
                    dim1 = static_cast<T>(0u);
                    dim2++;
                }
                return *this;
            }

            iterator operator++(int) {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const iterator& other) const {
                // std::cout << this->dim1 << 'x' << this->dim2 << " vs "
                //     << other.dim1 << 'x' << other.dim2
                //     << std::endl;
                return dim1 == other.dim1
                    && dim1_cnt == other.dim1_cnt
                    && dim2 == other.dim2
                    && dim2_cnt == other.dim2_cnt;
            }

            bool operator==(const sentinel&) const {
                return dim1 == 0u
                    && dim2 == dim2_cnt;
            }
        };

        auto begin() noexcept {
            return iterator(
                static_cast<T>(0u),
                dim1_cnt,
                static_cast<T>(0u),
                dim2_cnt
            );
        }

        auto end() const noexcept {
            return sentinel{};
        }
    };

    // Proper placement of the deduction guide
    template <std::integral T>
    pair_counter_view(T&&, T&&) -> pair_counter_view<T>;

    struct pair_counter_range_adaptor {
        template<std::integral T>
        constexpr auto operator() (T dim1_cnt, T dim2_cnt) const {
            return pair_counter_view<T>(dim1_cnt, dim2_cnt);
        }
    };

    template<std::integral T, typename Adaptor>
    auto operator|(const pair_counter_view<T>& pcv, Adaptor&& adaptor) {
        return adaptor(std::views::all(pcv));
    }

    template<std::integral T, typename Adaptor>
    auto operator|(pair_counter_view<T>&& pcv, Adaptor&& adaptor) {
        return adaptor(std::views::all(std::move(pcv)));
    }

    // static_assert(std::ranges::viewable_range<pair_counter_view<int>>);
    static_assert(std::movable<pair_counter_view<int>>);
    static_assert(std::ranges::view<pair_counter_view<int>>);










    template<typename T>
    struct type_displayer;

    template<typename ELEM, std::size_t cnt, bool error_check>
    struct static_vector_typer;

    template<typename ELEM, std::size_t cnt>
    struct static_vector_typer<ELEM, cnt, true> {
        using type = boost::container::static_vector<ELEM, cnt>;
    };

    template<typename ELEM, std::size_t cnt>
    struct static_vector_typer<ELEM, cnt, false> {
        using type = boost::container::static_vector<
            ELEM,
            cnt,
            boost::container::static_vector_options<
                boost::container::throw_on_overflow<false>,
                boost::container::inplace_alignment<0u>
            >::type
        >;
    };

    template<typename T>
    concept integral_or_floating_point = std::integral<T> || std::floating_point<T>;

    template<integral_or_floating_point T>
    struct constants {
        constexpr static T _0 { static_cast<T>(0) };
        constexpr static T _1 { static_cast<T>(1) };
    };


    // Can't use std::ranges::max_element because it requires a forward iterator
    // Can't use std::max_element because it requires a forward iterator
    template<std::input_iterator IT, std::sentinel_for<IT> S>
    IT max_element(IT first, S last, std::function<bool(const decltype(*first), const decltype(*first))> comp)  = delete;  // Wrap in std::ranges::common_view so .begin() and .end() return same type (no sentinel)

    template<std::input_iterator IT>
    IT max_element(IT first, IT last, std::function<bool(const decltype(*first), const decltype(*first))> comp) {
        // type_displayer<decltype(first)> x{};
        // type_displayer<decltype(last)> y{};
        if (first == last)
            return last;
        IT largest = first;
        while(++first != last) {
            if (comp(*largest, *first)) {
                largest = first;
            }
        }
        return largest;
    }
}

#endif //OFFBYNULL_UTILS_H
