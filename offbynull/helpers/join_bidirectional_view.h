#ifndef OFFBYNULL_HELPERS_JOIN_BIDIRECTIONAL_VIEW_H
#define OFFBYNULL_HELPERS_JOIN_BIDIRECTIONAL_VIEW_H

#include <ranges>
#include <iterator>
#include <utility>
#include <optional>
#include <type_traits>
#include "offbynull/utils.h"
#include "offbynull/concepts.h"
#include "offbynull/helpers/movable_box.h"

namespace offbynull::helpers::join_bidirectional_view {
    using offbynull::concepts::bidirectional_range_of_bidirectional_range;
    using offbynull::helpers::movable_box::movable_box;

    /** Beginning of range "flag" type, used for constructor overloading. */
    struct begin_marker {};
    /** End of range "flag" type, used for constructor overloading. */
    struct end_marker {};

    /**
     * Encapsulation of @ref offbynull::helpers::join_bidirectional_view::iterator's outer iterator.
     *
     * @tparam OUTER_IT Outer iterator type.
     */
    template <
        std::bidirectional_iterator OUTER_IT
    >
    struct outer_pack {
        /** Iterator at beginning position. */
        OUTER_IT it_begin;
        /** Iterator at end position. */
        OUTER_IT it_end;
        /** Iterator at current position. */
        OUTER_IT it;

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::outer_pack instance.
         *
         * @param it_begin_ Iterator at beginning position.
         * @param it_end_ Iterator at end position.
         * @param it_ Iterator at current position.
         */
        outer_pack(OUTER_IT& it_begin_, OUTER_IT& it_end_, OUTER_IT& it_)
        : it_begin { it_begin_ }
        , it_end { it_end_ }
        , it { it_ } {}

        /**
         * Equivalent to invoking `outer_pack {outer_it_begin, outer_it_end, outer_it_begin}`.
         *
         * @param outer_it_begin Iterator at beginning position.
         * @param outer_it_end Iterator at end position.
         * @return @ref offbynull::helpers::join_bidirectional_view::outer_pack instance with the current position at the beginning.
         */
        static std::optional<outer_pack> from_begin(OUTER_IT outer_it_begin, OUTER_IT outer_it_end) {
            return { { outer_it_begin, outer_it_end, outer_it_begin } };
        }

        /**
         * Equivalent to invoking `outer_pack {outer_it_begin, outer_it_end, outer_it_end}`.
         *
         * @param outer_it_begin Iterator at beginning position.
         * @param outer_it_end Iterator at end position.
         * @return @ref offbynull::helpers::join_bidirectional_view::outer_pack instance with the current position at the end.
         */
        static std::optional<outer_pack> from_end(OUTER_IT outer_it_begin, OUTER_IT outer_it_end) {
            return { { outer_it_begin, outer_it_end, outer_it_end } };
        }
    };

    /**
     * Encapsulation of @ref offbynull::helpers::join_bidirectional_view::iterator's inner range and iterator.
     *
     * @tparam OUTER_IT Outer iterator type.
     */
    template <std::bidirectional_iterator OUTER_IT>
    struct inner_pack {
        /** Inner range type. */
        using INNER_R = std::iter_value_t<OUTER_IT>;
        /** Inner range's iterator type. */
        using INNER_IT = std::ranges::iterator_t<INNER_R>;

        /** Copy of the inner range within the outer range. */
        INNER_R range;
        /** Iterator at beginning position of `range`. */
        INNER_IT it_begin;
        /** Iterator at end position of `range`. */
        INNER_IT it_end;
        /** Iterator at current position of `range`. */
        INNER_IT it;

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::inner_pack instance that holds a copy of the range at `outer_it`'s
         * current position and points to that beginning of that range.
         *
         * @param outer_it Outer iterator.
         */
        inner_pack(OUTER_IT& outer_it, begin_marker /*unused*/)
        : range { *outer_it }
        , it_begin { range.begin() }
        , it_end { range.end() }
        , it { it_begin } {}


        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::inner_pack instance that holds a copy of the range at `outer_it`'s
         * current position and points to that end of that range.
         *
         * @param outer_it Outer iterator.
         */
        inner_pack(OUTER_IT& outer_it, end_marker /*unused*/)
        : range { *outer_it }
        , it_begin { range.begin() }
        , it_end { range.end() }
        , it { it_end } {}

        // Can't have default implementation of copy consturctor/assignment because, even though range is copied and iterators are copied,
        // the iterator copies will point the original range as opposed to the new range.
        /**
         * Copy constructor.
         *
         * @param src Instance to copy.
         */
        inner_pack(const inner_pack& src)
        : range { src.range }
        , it_begin { range.begin() }
        , it_end { range.end() }
        , it { it_begin } {
            std::advance(it, std::ranges::distance(src.it_begin, src.it));  // Move it by appropriate amount
        }

        // Can't have default implementation of move consturctor/assignment because, even though range is moved and iterators are moved,
        // the moved iterator will probably still point to the original range in some way as opposed to the new range? Using default
        // implementation is causing use-after-destroyed in some places so custom implementation being used here.
        /**
         * Move constructor.
         *
         * @param src Instance to move.
         */
        inner_pack(inner_pack&& src) noexcept
        requires (!std::is_reference_v<INNER_R> && std::is_move_constructible_v<INNER_R>)  // Is INNER_R a ref? there's no object to move to
        : range { std::move(src.range) }
        , it_begin { range.begin() }
        , it_end { range.end() }
        , it { it_begin } {
            std::advance(it, std::ranges::distance(src.it_begin, src.it));  // Move it by appropriate amount
        }

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::inner_pack instance that's initialized to the beginning iterator
         * of the outer range's first item.
         *
         * @param outer_it_begin Beginning iterator of outer range.
         * @param outer_it_end Ending iterator of outer range.
         * @return Newly created @ref offbynull::helpers::join_bidirectional_view::inner_pack pointing at the beginning, or
         * `std::nullopt` if the outer range is empty.
         */
        static std::optional<inner_pack> from_begin(OUTER_IT outer_it_begin, OUTER_IT outer_it_end) {
            if (outer_it_begin == outer_it_end) {
                return { std::nullopt };
            }
            return { { outer_it_begin, begin_marker {} } };
        }

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::inner_pack instance that's initialized to the ending iterator
         * of the outer range's last item.
         *
         * @param outer_it_begin Beginning iterator of outer range.
         * @param outer_it_end Ending iterator of outer range.
         * @return Newly created @ref offbynull::helpers::join_bidirectional_view::inner_pack pointing at the end, or `std::nullopt` if
         * the outer range is empty.
         */
        static std::optional<inner_pack> from_end(OUTER_IT outer_it_begin, OUTER_IT outer_it_end) {
            if (outer_it_begin == outer_it_end) {
                return { std::nullopt };
            }
            OUTER_IT outer_it { outer_it_end };
            --outer_it;
            return { { outer_it, end_marker {} } };
        }
    };

    /**
     * Iterator for @ref offbynull::helpers::join_bidirectional_view::join_bidirectional_view.
     *
     * Note that, to avoid dangling references, this iterator returns copies instead of references. This is because this implementation is
     * structured such that it only holds on to the inner range currently being walked (by value, not by reference), meaning that once that
     * inner range is swapped for the next/previous inner range, the values returned are still valid. That wouldn't be the case if
     * references were returned (reference would be dangling once the inner range is destroyed).
     *
     * @tparam R Outer range type.
     */
    template <std::ranges::bidirectional_range R, bool const_>
    class iterator {
    private:
        using OUTER_R = std::conditional_t<const_, const R, R>;
        using OUTER_IT = std::ranges::iterator_t<OUTER_R>;
        using INNER_R = std::iter_reference_t<OUTER_IT>;
        using INNER_IT = std::ranges::iterator_t<INNER_R>;

        movable_box<std::optional<outer_pack<OUTER_IT>>> outer_boxed;
        movable_box<std::optional<inner_pack<OUTER_IT>>> inner_boxed;  // Set to nullopt if !outer.has_value() || outer->it == outer->it_end

        void skip_empty_ranges_forward() {
            if (!outer_boxed.get().has_value()) {
                return;  // No outer range
            }
            while (outer_boxed.get()->it != outer_boxed.get()->it_end) {
                if (inner_boxed.get()->it_begin == inner_boxed.get()->it_end) {
                    ++outer_boxed.get()->it;
                    if (outer_boxed.get()->it != outer_boxed.get()->it_end) {
                        inner_boxed = std::optional<inner_pack<OUTER_IT>> { { inner_pack(outer_boxed.get()->it, begin_marker {}) } };
                    } else {
                        inner_boxed = std::optional<inner_pack<OUTER_IT>> { { std::nullopt } };
                    }
                } else {
                    break;
                }
            }
        }

        void skip_empty_ranges_backward() {
            if (!outer_boxed.get().has_value()) {
                return;
            }
            OUTER_IT outer_it { outer_boxed.get()->it };
            std::optional<inner_pack<OUTER_IT>> inner_ = inner_boxed.get();
            do {
                if (!inner_.has_value() || inner_->it_begin == inner_->it_end) {
                    --outer_it;
                    inner_.emplace(inner_pack(outer_it, end_marker {}));
                } else {
                    break;
                }
            } while (outer_it != outer_boxed.get()->it_begin);

            if (outer_it != outer_boxed.get()->it) {
                outer_boxed.get()->it = outer_it;
                inner_boxed = inner_;
            }
        }

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::iter_value_t<INNER_IT>;
        using difference_type = std::iter_difference_t<INNER_IT>;
        using reference = value_type;  // DO NOT USE std::iter_reference_t<INNER_IT> - this iterator must always return value type because
                                       // once it goes past the inner range the reference will be dangling

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::iterator instance that has no elements.
         */
        iterator()
        : outer_boxed {}
        , inner_boxed {} {}

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::iterator at the beginning.
         */
        iterator(OUTER_IT outer_it_begin_, OUTER_IT outer_it_end_, begin_marker /*unused*/)
        : outer_boxed { outer_pack<OUTER_IT>::from_begin(outer_it_begin_, outer_it_end_) }
        , inner_boxed { inner_pack<OUTER_IT>::from_begin(outer_boxed.get()->it_begin, outer_boxed.get()->it_end) } {
            skip_empty_ranges_forward();
        }

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::iterator at the end (one past the last element).
         */
        iterator(OUTER_IT outer_it_begin_, OUTER_IT outer_it_end_, end_marker /*unused*/)
        : outer_boxed { outer_pack<OUTER_IT>::from_end(outer_it_begin_, outer_it_end_) }
        , inner_boxed { std::nullopt } {}

        iterator(const iterator<R, const_> &src) = default;
        iterator(iterator<R, const_> &&src) noexcept = default;
        iterator<R, const_>& operator=(const iterator<R, const_>& rhs) = default;

        value_type operator*() const {
            return *inner_boxed.get()->it;
        }

        iterator& operator++() {
            ++inner_boxed.get()->it;
            if (inner_boxed.get()->it == inner_boxed.get()->it_end) {
                ++outer_boxed.get()->it;
                if (outer_boxed.get()->it != outer_boxed.get()->it_end) {
                    inner_boxed = std::optional<inner_pack<OUTER_IT>> { { outer_boxed.get()->it, begin_marker {} } };
                    skip_empty_ranges_forward();
                } else {
                    inner_boxed = std::optional<inner_pack<OUTER_IT>> { std::nullopt };
                }
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator& operator--() {
            if (!inner_boxed.get().has_value() || inner_boxed.get()->it == inner_boxed.get()->it_begin) {
                --outer_boxed.get()->it;
                inner_boxed = std::optional<inner_pack<OUTER_IT>> { { outer_boxed.get()->it, end_marker {} } };
                skip_empty_ranges_backward();
            }
            --inner_boxed.get()->it;
            return *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            // If no outer, always return true.
            if (!outer_boxed.get().has_value()) {
                return true;
            }
            // Because inner is always going to be a copy, directly returning ...
            //
            //   outer_it == other.outer_it && inner.it == other.inner.it
            //
            // ... won't work. Since this's inner and other's inner are copies sof each other, their iterator equality will always be
            // false. To work around this, first check if both this and other have the same outer between them (if yes, their outer_its will
            // be equal because both outer_its will be pointing to the same collection).
            if (outer_boxed.get()->it_begin != other.outer_boxed.get()->it_begin) {
                return false;  // Different outer objects detected
            }
            // Then, check if both are ended. This is the most important case to handle, so it should be performant.
            bool this_ended { outer_boxed.get()->it == outer_boxed.get()->it_end };
            bool other_ended { other.outer_boxed.get()->it == other.outer_boxed.get()->it_end };
            if (other_ended) {
                return this_ended;
            } else if (this_ended) {
                return other_ended;
            }
            // If not ended, get the distance between the inner iterators and test that instead. This will always work, but depending on the
            // type of inner.range / inner.it, it may not be performant (should be fine because this case will almost never be hit).
            if (outer_boxed.get()->it != other.outer_boxed.get()->it) {  // Both at same outer position?
                return false;
            }
            if (inner_boxed.get().has_value() != other.inner_boxed.get().has_value()) {  // Both have inner range?
                return false;
            }
            return std::ranges::distance(inner_boxed.get()->it_begin, inner_boxed.get()->it)
                    == std::ranges::distance(other.inner_boxed.get()->it_begin, other.inner_boxed.get()->it);
        }
    };

    /**
     * Bidirectional equivalent of STL `join_view`. In other words, wraps a nested bidirectional range (bidirectional range of
     * bidirectional ranges) as if a single flattened bidirectional range.
     *
     * Note that, even if the underlying inner range returns references, this range returns values (copies). This is done to avoid
     * dangling references: This implementation is structured such that it only holds on to the inner range currently being walked (by
     * value, not by reference), meaning that once that inner range is swapped for the next/previous inner range, the values returned are
     * still valid. That wouldn't be the case if references were returned (reference would be dangling once the inner range is destroyed).
     *
     * @tparam R Type of nested bidirectional range.
     */
    template<bidirectional_range_of_bidirectional_range R>
    class join_bidirectional_view : public std::ranges::view_interface<join_bidirectional_view<R>> {
    private:
        R r;
    public:
        /** Iterator type. */
        using I = iterator<R, false>;
        /** Constant iterator type. */
        using IC = iterator<R, true>;

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::join_bidirectional_view instance.
         *
         * @param r_ Range to wrap.
         */
        join_bidirectional_view(R&& r_)
        : r(std::forward<R>(r_)) {}

        join_bidirectional_view(const join_bidirectional_view<R> &src) = default;
        join_bidirectional_view(join_bidirectional_view<R> &&src) noexcept = default;
        join_bidirectional_view<R>& operator=(const join_bidirectional_view<R>& other) = default;
        join_bidirectional_view<R>& operator=(join_bidirectional_view<R>&& other) noexcept = default;


        /**
         * Get begin iterator.
         *
         * @return begin iterator.
         */
        I begin() {
            return { std::ranges::begin(r), std::ranges::end(r), begin_marker {} };
        }
        /**
         * Get end iterator.
         *
         * @return end iterator.
         */
        I end() requires std::ranges::common_range<R> {
            return { std::ranges::begin(r), std::ranges::end(r), end_marker {} };
        }

        /**
         * Get begin iterator.
         *
         * @return begin iterator (const).
         */
        IC begin() const {
            return { std::ranges::begin(r), std::ranges::end(r), begin_marker {} };
        }
        /**
         * Get end iterator.
         *
         * @return end iterator (const).
         */
        IC end() const requires std::ranges::common_range<R> {
            return { std::ranges::begin(r), std::ranges::end(r), end_marker {} };
        }
    };

    template<typename R>
    join_bidirectional_view(R&&) -> join_bidirectional_view<std::remove_cvref_t<R>>;

    struct join_bidirectional_closure {
        template<typename R>
        auto operator()(R&& r) const {
            using V = std::views::all_t<R>;
            return join_bidirectional_view<V> { std::views::all(std::forward<R>(r)) };
        }

        template<typename R>
        friend auto operator|(R&& r, const join_bidirectional_closure& c) {
            return c(std::forward<R>(r));
        }
    };

    inline auto join_bidirectional() {
        return join_bidirectional_closure {};
    }

    template<typename R>
    auto join_bidirectional(R&& r) {
        return join_bidirectional_closure {}(std::forward<R>(r));
    }
}

#endif //OFFBYNULL_HELPERS_JOIN_BIDIRECTIONAL_VIEW_H