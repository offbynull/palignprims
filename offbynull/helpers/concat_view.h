#ifndef OFFBYNULL_HELPERS_CONCAT_VIEW_H
#define OFFBYNULL_HELPERS_CONCAT_VIEW_H

#include <ranges>
#include <vector>
#include <offbynull/utils.h>

namespace offbynull::helpers::concat_view {
    class sentinel {
    public:
        // need to provide sentinel == iterator separately
        bool operator==(const auto& i) const {
            return i == *this;
        }
    };

    // https://www.reddit.com/r/cpp_questions/comments/1cp0rwu/how_to_lazily_concatenate_two_views/
    template <
        std::bidirectional_iterator It1,
        std::sentinel_for<It1> S1,
        std::bidirectional_iterator It2,
        std::sentinel_for<It2> S2
    >
    requires std::same_as<decltype(*std::declval<It1>()), decltype(*std::declval<It2>())>
    class iterator {
        It1 begin1;
        It1 it1;
        S1 end1;
        It2 begin2;
        It2 it2;
        S2 end2;
        bool is_first;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = typename It1::value_type;
        // using reference = typename It1::reference;
        using iterator_category = std::bidirectional_iterator_tag;

        iterator(const iterator<It1, S1, It2, S2> &src) = default;

        iterator(iterator<It1, S1, It2, S2> &&src) = default;

        iterator()
        : begin1 {}
        , it1 {}
        , end1 {}
        , begin2 {}
        , it2 {}
        , end2 {}
        , is_first { false } {}

        iterator(It1 a_first, S1 a_last, It2 b_first, S2 b_last)
        : begin1(a_first)
        , it1(a_first)
        , end1(a_last)
        , begin2(b_first)
        , it2(b_first)
        , end2(b_last)
        , is_first(a_first == a_last ? false : true) {}

        iterator<It1, S1, It2, S2>& operator=(const iterator<It1, S1, It2, S2>& rhs) = default;

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

        bool operator==(const sentinel&) const {
            return !is_first && it2 == end2;
        }
    };

    // This should be using std::views::concat, but it wasn't included in this version of the C++ standard
    // library. The concat implementation below lacks several features (e.g. doesn't support the pipe operator)
    // and forcefully returns copies (concat_view::iterator::value_type ==
    // concat_view::iterator::reference_type).
    template <std::ranges::forward_range R1, std::ranges::forward_range R2>
    class concat_view : public std::ranges::view_interface<concat_view<R1, R2>> {
    private:
        R1 first_range;
        R2 second_range;

    public:
        concat_view(R1&& first, R2&& second)
        : first_range(std::forward<R1>(first))
        , second_range(std::forward<R2>(second)) {}

        concat_view(const concat_view<R1, R2>& other) = default;
        concat_view(concat_view<R1, R2>&& other) = default;
        concat_view<R1, R2>& operator=(const concat_view<R1, R2>& other) = default;
        concat_view<R1, R2>& operator=(concat_view<R1, R2>&& other) = default;

        std::bidirectional_iterator auto begin() {
            return iterator(
                std::ranges::begin(first_range),
                std::ranges::end(first_range),
                std::ranges::begin(second_range),
                std::ranges::end(second_range)
            );
        }

        sentinel end() noexcept { return {}; }
    };

    // Proper placement of the deduction guide
    template <std::ranges::forward_range R1, std::ranges::forward_range R2>
    concat_view(R1&&, R2&&) -> concat_view<R1, R2>;

    struct concat_range_adaptor {
        template<std::ranges::forward_range R1, std::ranges::forward_range R2>
        constexpr auto operator() (R1&& r1, R2&& r2) const {
            return concat_view<R1, R2>(std::forward<R1>(r1), std::forward<R2>(r2));
        }
    };

    template<std::ranges::forward_range R1, std::ranges::forward_range R2, typename Adaptor>
    auto operator|(const concat_view<R1, R2>& r, Adaptor adaptor) {
        return adaptor(std::views::all(r));
    }

    template<std::ranges::forward_range R1, std::ranges::forward_range R2, typename Adaptor>
    auto operator|(concat_view<R1, R2>&& r, Adaptor adaptor) {
        return adaptor(std::views::all(std::move(r)));  // doing std::views::all(std::move(r)) causes compile error -- all() doesn't accept rvalue refs?
    }
}

#endif //OFFBYNULL_HELPERS_CONCAT_VIEW_H