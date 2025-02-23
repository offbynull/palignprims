#ifndef OFFBYNULL_HELPERS_JOIN_BIDIRECTIONAL_VIEW_H
#define OFFBYNULL_HELPERS_JOIN_BIDIRECTIONAL_VIEW_H

#include <ranges>
#include <iterator>
#include <cstdint>
#include <utility>
#include <optional>
#include <type_traits>
#include "offbynull/utils.h"
#include "offbynull/concepts.h"

/**
 * View that flattens a nested bidirectional range, similar to STL `join_view` but supports bidirectionality.
 *
 * @author Kasra Faghihi
 */
namespace offbynull::helpers::join_bidirectional_view {
    using offbynull::concepts::bidirectional_range_of_bidirectional_range;

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
    template <
        std::bidirectional_iterator OUTER_IT
    >
    struct inner_pack {
        /** Inner range type. */
        using INNER_R = std::remove_cvref_t<decltype(*std::declval<OUTER_IT>())>;
        /** Inner range's iterator type. */
        using INNER_IT = std::remove_cvref_t<decltype(std::declval<INNER_R>().begin())>;

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

        /**
         * Copy assignment.
         *
         * @param rhs Instance to copy.
         * @return Self.
         */
        inner_pack& operator=(const inner_pack &rhs) noexcept {
            range = rhs.range;
            it_begin = range.begin();
            it_end = range.end();
            it = it_begin;
            std::advance(it, std::ranges::distance(rhs.it_begin, rhs.it));  // Move it by appropriate amount
            return *this;
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
        : range { std::move(src.range) }
        , it_begin { range.begin() }
        , it_end { range.end() }
        , it { it_begin } {
            std::advance(it, std::ranges::distance(src.it_begin, src.it));  // Move it by appropriate amount
        }

        /**
         * Move assignment.
         *
         * @param rhs Instance to move.
         * @return Self.
         */
        inner_pack& operator=(inner_pack &&rhs) noexcept {
            range = std::move(rhs.range);
            it_begin = range.begin();
            it_end = range.end();
            it = it_begin;
            std::advance(it, std::ranges::distance(rhs.it_begin, rhs.it));  // Move it by appropriate amount
            return *this;
        }

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::inner_pack instance that's initialized to the beginning iterator
         * of the outer range's first item.
         *
         * @param outer_it_begin Beginning iterator of outer range.
         * @param outer_it_end Ending iterator of outer range.
         * @return Newly created @ref offbynull::helpers::join_bidirectional_view::inner_pack pointing at the beginning, or
         * @ref stdd:nullopt if the outer range is empty.
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
         * @return Newly created @ref offbynull::helpers::join_bidirectional_view::inner_pack pointing at the end, or @ref std:nullopt if
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
     * @tparam OUTER_IT Outer iterator type.
     */
    template <
        std::bidirectional_iterator OUTER_IT
    >
    class iterator {
    private:
        using INNER_R = std::remove_cvref_t<decltype(*std::declval<OUTER_IT>())>;
        using INNER_IT = std::remove_cvref_t<decltype(std::declval<INNER_R>().begin())>;

        std::optional<outer_pack<OUTER_IT>> outer;
        std::optional<inner_pack<OUTER_IT>> inner;  // Set to nullopt if !outer.has_value() || outer->it == outer->it_end

        void skip_empty_ranges_forward() {
            if (!outer.has_value()) {
                return;  // No outer range
            }
            while (outer->it != outer->it_end) {
                if (inner->it_begin == inner->it_end) {
                    ++outer->it;
                    if (outer->it != outer->it_end) {
                        inner = { inner_pack(outer->it, begin_marker {}) };
                    } else {
                        inner = { std::nullopt };
                    }
                } else {
                    break;
                }
            }
        }

        void skip_empty_ranges_backward() {
            if (!outer.has_value()) {
                return;
            }
            OUTER_IT outer_it { outer->it };
            std::optional<inner_pack<OUTER_IT>> inner_ = inner;
            do {
                if (!inner_.has_value() || inner_->it_begin == inner_->it_end) {
                    --outer_it;
                    inner_ = { inner_pack(outer_it, end_marker {}) };
                } else {
                    break;
                }
            } while (outer_it != outer->it_begin);

            if (outer_it != outer->it) {
                outer->it = outer_it;
                inner = inner_;
            }
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::remove_cvref_t<typename INNER_IT::value_type>;
        // using reference = typename INNER_IT::reference;
        using iterator_category = std::bidirectional_iterator_tag;

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::iterator instance that has no elements.
         */
        iterator()
        : outer { std::nullopt }
        , inner { std::nullopt } {}

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::iterator at the beginning.
         */
        iterator(OUTER_IT outer_it_begin_, OUTER_IT outer_it_end_, begin_marker /*unused*/)
        : outer { outer_pack<OUTER_IT>::from_begin(outer_it_begin_, outer_it_end_) }
        , inner { inner_pack<OUTER_IT>::from_begin(outer->it_begin, outer->it_end) } {
            skip_empty_ranges_forward();
        }

        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::iterator at the end (one past the last element).
         */
        iterator(OUTER_IT outer_it_begin_, OUTER_IT outer_it_end_, end_marker /*unused*/)
        : outer { outer_pack<OUTER_IT>::from_end(outer_it_begin_, outer_it_end_) }
        , inner { std::nullopt } {}

        iterator(const iterator<OUTER_IT> &src) = default;
        iterator(iterator<OUTER_IT> &&src) noexcept = default;
        iterator<OUTER_IT>& operator=(const iterator<OUTER_IT>& rhs) = default;

        value_type operator*() const {
            return *inner->it;
        }

        iterator& operator++() {
            ++inner->it;
            if (inner->it == inner->it_end) {
                ++outer->it;
                if (outer->it != outer->it_end) {
                    inner = { outer->it, begin_marker {} };
                    skip_empty_ranges_forward();
                } else {
                    inner = { std::nullopt };
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
            if (!inner.has_value() || inner->it == inner->it_begin) {
                --outer->it;
                inner = { outer->it, end_marker {} };
                skip_empty_ranges_backward();
            }
            --inner->it;
            return *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            // If no outer, always return true.
            if (!outer.has_value()) {
                return true;
            }
            // Because inner is always going to be a copy, directly returning ...
            //
            //   outer_it == other.outer_it && inner.it == other.inner.it
            //
            // ... won't work. Since this's inner and other's inner are copies sof each other, their iterator equality will always be
            // false. To work around this, first check if both this and other have the same outer between them (if yes, their outer_its will
            // be equal because both outer_its will be pointing to the same collection).
            if (outer->it_begin != other.outer->it_begin) {
                return false;  // Different outer objects detected
            }
            // Then, check if both are ended. This is the most important case to handle, so it should be performant.
            bool this_ended { outer->it == outer->it_end };
            bool other_ended { other.outer->it == other.outer->it_end };
            if (other_ended) {
                return this_ended;
            } else if (this_ended) {
                return other_ended;
            }
            // If not ended, get the distance between the inner iterators and test that instead. This will always work, but depending on the
            // type of inner.range / inner.it, it may not be performant (should be fine because this case will almost never be hit).
            if (outer->it != other.outer->it) {  // Both at same outer position?
                return false;
            }
            if (inner.has_value() != other.inner.has_value()) {  // Both have inner range?
                return false;
            }
            return std::ranges::distance(inner->it_begin, inner->it) == std::ranges::distance(other.inner->it_begin, other.inner->it);
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
        R range;

    public:
        /**
         * Construct an @ref offbynull::helpers::join_bidirectional_view::join_bidirectional_view.
         *
         * @param range_ Backing range (copied / moved),
         */
        join_bidirectional_view(R&& range_)
        : range(std::forward<R>(range_)) {}

        join_bidirectional_view(const join_bidirectional_view<R>& other) = default;
        join_bidirectional_view(join_bidirectional_view<R>&& other) noexcept = default;
        join_bidirectional_view<R>& operator=(const join_bidirectional_view<R>& other) = default;
        join_bidirectional_view<R>& operator=(join_bidirectional_view<R>&& other) = default;

        std::bidirectional_iterator auto begin() {
            return iterator(
                std::ranges::begin(range),
                std::ranges::end(range),
                begin_marker {}
            );
        }

        std::bidirectional_iterator auto end() {
            return iterator(
                std::ranges::begin(range),
                std::ranges::end(range),
                end_marker {}
            );
        }
    };

    /**
     * @ref offbynull::helpers::join_bidirectional_view::join_bidirectional_view template deduction guide.
     *
     * @tparam R Type backing @ref offbynull::helpers::join_bidirectional_view::join_bidirectional_view.
     * @return No return (this is a template deduction guide).
     */
    template<bidirectional_range_of_bidirectional_range R>
    join_bidirectional_view(R&&) -> join_bidirectional_view<R>;

    /**
     * @ref offbynull::helpers::join_bidirectional_view::join_bidirectional_view range pipe adaptor, enabling some range to be fed into
     * this view via a pipe operator (e.g. `r | join_bidirectional_view_adaptor{}`).
     *
     * @tparam R Type backing @ref offbynull::helpers::join_bidirectional_view::join_bidirectional_view.
     * @return `r` piped into `adaptor`.
     */
    struct join_bidirectional_view_adaptor {
        template<bidirectional_range_of_bidirectional_range R>
        constexpr auto operator()(R&& r) const {
            return join_bidirectional_view<R>(std::forward<R>(r));
        }
    };

    /**
     * @ref offbynull::helpers::join_bidirectional_view::join_bidirectional_view range pipe operator, enabling the view to be fed into
     * another range adaptor (e.g. `join_bidirectional_view {r} | std::views::transform([](const auto& n) { return n*2; })`).
     *
     * @tparam R Type backing @ref offbynull::helpers::join_bidirectional_view::join_bidirectional_view.
     * @param r Range to pipe from.
     * @param adaptor Range adaptor to pipe into.
     * @return `r` piped into `adaptor`.
     */
    template<bidirectional_range_of_bidirectional_range R>
    auto operator|(R&& r, join_bidirectional_view_adaptor const& adaptor) {
        return adaptor(std::forward<R>(r));
    }
}

#endif //OFFBYNULL_HELPERS_JOIN_BIDIRECTIONAL_VIEW_H