#ifndef OFFBYNULL_HELPERS_CONCAT_BIDIRECTIONAL_VIEW_H
#define OFFBYNULL_HELPERS_CONCAT_BIDIRECTIONAL_VIEW_H

#include <ranges>
#include <iterator>
#include <utility>
#include <type_traits>
#include "offbynull/utils.h"

namespace offbynull::helpers::concat_bidirectional_view {
    using offbynull::utils::wider_numeric;

    struct begin_marker {};
    struct end_marker {};

    // https://www.reddit.com/r/cpp_questions/comments/1cp0rwu/how_to_lazily_concatenate_two_views/
    template <
        std::ranges::bidirectional_range R1,
        std::ranges::bidirectional_range R2,
        bool const_
    >
    class iterator {
        using R1_ = std::conditional_t<const_, const R1, R1>;
        using I1 = std::ranges::iterator_t<R1_>;
        using S1 = std::ranges::sentinel_t<R1_>;
        using R2_ = std::conditional_t<const_, const R2, R2>;
        using I2 = std::ranges::iterator_t<R2_>;
        using S2 = std::ranges::sentinel_t<R2_>;

        I1 begin1;
        I1 it1;
        S1 end1;
        I2 begin2;
        I2 it2;
        S2 end2;
        bool is_first;

    public:
        using difference_type = wider_numeric<std::iter_difference_t<I1>, std::iter_difference_t<I2>>::type;
        using value_type = std::common_type_t<std::iter_value_t<I1>, std::iter_value_t<I2>>;
        using reference = std::common_reference_t<std::iter_reference_t<I1>, std::iter_reference_t<I2>>;
        using iterator_category = std::bidirectional_iterator_tag;

        iterator(const iterator &src) = default;
        iterator(iterator &&src) noexcept = default;

        iterator()
        : begin1 {}
        , it1 {}
        , end1 {}
        , begin2 {}
        , it2 {}
        , end2 {}
        , is_first { false } {}

        iterator(I1 a_first, S1 a_last, I2 b_first, S2 b_last, begin_marker)
        : begin1(a_first)
        , it1(a_first)
        , end1(a_last)
        , begin2(b_first)
        , it2(b_first)
        , end2(b_last)
        , is_first(a_first == a_last ? false : true) {}

        iterator(I1 a_first, S1 a_last, I2 b_first, S2 b_last, end_marker)
        : begin1(a_first)
        , it1(a_last)
        , end1(a_last)
        , begin2(b_first)
        , it2(b_last)
        , end2(b_last)
        , is_first(false) {}

        iterator& operator=(const iterator& rhs) = default;
        iterator& operator=(iterator&& rhs) noexcept = default;

        reference operator*() const {
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

        iterator& operator--() {
            if (!is_first) {
                if (it2 == begin2) {
                    is_first = true;
                    --it1;
                } else {
                    --it2;
                }
            } else {
                --it1;
            }
            return *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }


        bool operator==(const iterator& other) const {
            return is_first == other.is_first &&
                   (is_first ? it1 == other.it1 : it2 == other.it2);
        }
    };

    // This should be using std::views::concat, but it wasn't included in this version of the C++ standard
    // library. The concat implementation below lacks several features (e.g. doesn't support the pipe operator)
    // and forcefully returns copies (concat_bidirectional_view::iterator::value_type ==
    // concat_bidirectional_view::iterator::reference_type).
    template <std::ranges::bidirectional_range R1, std::ranges::bidirectional_range R2>
    class concat_bidirectional_view : public std::ranges::view_interface<concat_bidirectional_view<R1, R2>> {
    private:
        R1 first_range;
        R2 second_range;

    public:
        /** Iterator type. */
        using I = iterator<R1, R2, false>;
        /** Iterator sentinel type. */
        using S = iterator<R1, R2, false>;
        /** Constant iterator type. */
        using IC = iterator<R1, R2, true>;
        /** Constant iterator sentinel type. */
        using SC = iterator<R1, R2, true>;

        concat_bidirectional_view(R1&& first, R2&& second)
        : first_range(std::forward<R1>(first))
        , second_range(std::forward<R2>(second)) {}

        concat_bidirectional_view(const concat_bidirectional_view<R1, R2>& other) = default;
        concat_bidirectional_view(concat_bidirectional_view<R1, R2>&& other) noexcept = default;
        concat_bidirectional_view<R1, R2>& operator=(const concat_bidirectional_view<R1, R2>& other) = default;
        concat_bidirectional_view<R1, R2>& operator=(concat_bidirectional_view<R1, R2>&& other) = default;

        /**
         * Get begin iterator.
         *
         * @return begin iterator.
         */
        I begin() {
            return {
                std::ranges::begin(first_range),
                std::ranges::end(first_range),
                std::ranges::begin(second_range),
                std::ranges::end(second_range),
                begin_marker {}
            };
        }
        /**
         * Get end iterator.
         *
         * @return end iterator.
         */
        I end()  requires std::ranges::common_range<R1> && std::ranges::common_range<R2> {
            return {
                std::ranges::begin(first_range),
                std::ranges::end(first_range),
                std::ranges::begin(second_range),
                std::ranges::end(second_range),
                end_marker {}
            };
        }

        /**
         * Get begin iterator.
         *
         * @return begin iterator (const).
         */
        IC begin() const {
            return {
                std::ranges::begin(first_range),
                std::ranges::end(first_range),
                std::ranges::begin(second_range),
                std::ranges::end(second_range),
                begin_marker {}
            };
        }
        /**
         * Get end iterator.
         *
         * @return end iterator (const).
         */
        IC end() const requires std::ranges::common_range<R1> && std::ranges::common_range<R2> {
            return {
                std::ranges::begin(first_range),
                std::ranges::end(first_range),
                std::ranges::begin(second_range),
                std::ranges::end(second_range),
                end_marker {}
            };
        }
    };

    /**
     * @ref offbynull::helpers::concat_bidirectional_view::concat_bidirectional_view template deduction guide.
     *
     * @tparam R1 First range type backing @ref offbynull::helpers::concat_bidirectional_view::concat_bidirectional_view.
     * @tparam R2 Second range type backing @ref offbynull::helpers::concat_bidirectional_view::concat_bidirectional_view.
     * @return No return (this is a template deduction guide).
     */
    template<std::ranges::bidirectional_range R1, std::ranges::bidirectional_range R2>
    concat_bidirectional_view(R1&&, R2&&) -> concat_bidirectional_view<R1, R2>;
}

#endif //OFFBYNULL_HELPERS_CONCAT_BIDIRECTIONAL_VIEW_H