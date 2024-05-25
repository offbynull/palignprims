#ifndef OFFBYNULL_UTILS_H
#define OFFBYNULL_UTILS_H

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

        class sentinel {
        public:
            // need to provide sentinel == iterator separately
            bool operator==(const auto& i) const {
                return i == *this;
            }
        };

        template <typename It1, typename S1, typename It2, typename S2>
        class iterator {
            It1 it1;
            S1 end1;
            It2 it2;
            S2 end2;
            bool isFirst;

           public:
            using difference_type = std::ptrdiff_t;
            using value_type = std::common_type_t<decltype(*std::declval<It1>()), decltype(*std::declval<It2>())>;
            using pointer = value_type*;
            using reference = value_type;
            using iterator_category = std::input_iterator_tag;

            iterator(It1 first, S1 last1, It2 second, S2 last2, bool firstRange)
                : it1(first),
                  end1(last1),
                  it2(second),
                  end2(last2),
                  isFirst(firstRange) {}

            reference operator*() {
                if (isFirst) {
                    if (it1 != end1) {
                        return *it1;
                    } else {
                        isFirst = false;
                        return *it2;
                    }
                } else {
                    return *it2;
                }
            }

            iterator& operator++() {
                if (isFirst) {
                    if (++it1 == end1) {
                        isFirst = false;
                    }
                } else {
                    ++it2;
                }
                return *this;
            }
            iterator& operator++(int) {
                ++(*this);
                return *this;
            }

            bool operator==(const iterator& other) const {
                return isFirst == other.isFirst &&
                       (isFirst ? it1 == other.it1 : it2 == other.it2);
            }

            bool operator==(const concat_view<R1, R2>::sentinel&) const {
                return !isFirst && it2 == end2;
            }

        };

        auto begin() {
            return iterator(
                std::ranges::begin(first_range), std::ranges::end(first_range),
                std::ranges::begin(second_range), std::ranges::end(second_range),
                true
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

    template<typename T>
    struct type_displayer;
}

#endif //OFFBYNULL_UTILS_H
