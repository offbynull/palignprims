#ifndef OFFBYNULL_ALIGNER_SCORERS_SINGLE_CHARACTER_SUBSTITUTION_MATRIX_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_SINGLE_CHARACTER_SUBSTITUTION_MATRIX_SCORER_H

#include <cstddef>
#include <tuple>
#include <ranges>
#include <optional>
#include <array>
#include <algorithm>
#include <string_view>
#include <type_traits>
#include <sstream>
#include <limits>
#include <utility>
#include <functional>
#include <stdexcept>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/utils.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::scorers::single_character_substitution_matrix_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::utils::type_displayer;
    using offbynull::concepts::widenable_to_size_t;

    /**
     * @ref offbynull::aligner::scorer::scorer::scorer which scores single character elements based on a space-delimited ASCII table.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam alphabet_size Number of characters (must be <= 255).
     * @tparam SEQ_INDEX Sequence indexer type.
     * @tparam WEIGHT Alignment graph's edge weight type.
     */
    template<bool debug_mode, std::size_t alphabet_size, widenable_to_size_t SEQ_INDEX, weight WEIGHT>
    class single_character_substitution_matrix_scorer {
    private:
        static_assert(alphabet_size <= 255zu, "Alphabet greater than 255 symbols");

        static auto trim_whitespace(const auto& text) {
            auto leading_trimmed {
                text
                | std::views::drop_while([](auto ch) { return ch == ' ' || ch == '\t' || ch == '\n'; })
            };
            auto trailing_trimmed {
                text
                | std::views::reverse
                | std::views::drop_while([](auto ch) { return ch == ' ' || ch == '\t' || ch == '\n'; })
                | std::views::reverse
            };
            const char& start_char_ref { *trailing_trimmed.begin() };
            const char* start_char_ptr { &start_char_ref }; // can't use line.begin() directly because line is an iterator object, not a
                                                            // pointer -- need to first grab the object from the iterator (which is a ref
                                                            // into a element in text) then get the pointer to it
            return std::string_view { start_char_ptr, static_cast<std::size_t>(std::ranges::distance(trailing_trimmed)) };
        }

        static auto split_lines_and_words(const auto& text) {
            auto text_normalized_ends {
                text
                | std::views::filter([](const auto& ch) { return ch != '\r'; })
            };
            return
                std::views::split(text_normalized_ends, '\n')
                | std::views::transform([](const auto& line) {
                    static_assert(std::is_same_v<decltype(*line.begin()), const char&>); // ensure you're getting back a REFERENCE to the
                                                                                         // original data when you access
                    const char& start_char_ref { *line.begin() };
                    const char* start_char_ptr { &start_char_ref }; // can't use line.begin() directly because line is an iterator object,
                                                                    // not a pointer -- need to first grab the object from the iterator
                                                                    // (which is a ref into a element in text) then get the pointer to it
                    return std::string_view { start_char_ptr, static_cast<std::size_t>(std::ranges::distance(line)) };
                })
                | std::views::transform([](const std::string_view& line) {
                    return trim_whitespace(line);  // trim whitespace from line
                })
                | std::views::filter([](const std::string_view& line) {
                    return line.begin() != line.end();  // remove if empty line
                })
                | std::views::transform([](const std::string_view& line) {
                    return
                        std::views::split(line, ' ')
                        | std::views::filter([](const auto& s) {
                            return s.begin() != s.end();
                        });
                });
        }

        static char extract_symbol(const auto& element_view) {
            auto element_view_it { element_view.begin() };
            if constexpr (debug_mode) {
                if (element_view_it == element_view.end()) {
                    throw std::runtime_error { "Expected 1 character for each alphabet item but 0 characters found" };
                }
            }
            char ret { *element_view_it };
            if constexpr (debug_mode) {
                ++element_view_it;
                if (element_view_it != element_view.end()) {
                    throw std::runtime_error { "Expected 1 character for each alphabet item but >1 characters found" };
                }
            }
            return ret;
        }

        static std::array<char, alphabet_size> extract_sorted_alphabet(const std::string_view& text) {
            // split test line-by-line
            auto lines { split_lines_and_words(text) };
            auto lines_it { lines.begin() };
            // pull out header and split on space
            auto header_words { *lines_it };
            // set alphabet
            if constexpr (debug_mode) {
                auto size { std::ranges::distance(header_words) };
                if (size != alphabet_size) {
                    throw std::runtime_error { "Unexpected number of characters" };
                }
            }
            std::array<char, alphabet_size> ret {};
            auto header_words_it { header_words.begin() };
            for (std::size_t i { 0zu }; i < alphabet_size; i++) {
                ret[i] = extract_symbol(*header_words_it);
                ++header_words_it;
            }
            std::ranges::sort(ret.begin(), ret.end());
            return ret;
        }

        static std::size_t to_weights_idx(
            const std::array<char, alphabet_size> sorted_alphabet,
            const char down_elem,
            const char right_elem
        ) {
            auto down_it { std::lower_bound(sorted_alphabet.begin(), sorted_alphabet.end(), down_elem) };
            if constexpr (debug_mode) {
                if (down_it == sorted_alphabet.end() || *down_it != down_elem) {
                    throw std::runtime_error { "Down element not in matrix" };
                }
            }
            auto right_it { std::lower_bound(sorted_alphabet.begin(), sorted_alphabet.end(), right_elem) };
            if constexpr (debug_mode) {
                if (right_it == sorted_alphabet.end() || *right_it != right_elem) {
                    throw std::runtime_error { "Right element not in matrix" };
                }
            }
            std::size_t down_idx { static_cast<std::size_t>(down_it - sorted_alphabet.begin()) };
            std::size_t right_idx { static_cast<std::size_t>(right_it - sorted_alphabet.begin()) };
            return down_idx * alphabet_size + right_idx;
        }

        static std::array<WEIGHT, alphabet_size * alphabet_size> extract_sorted_weights(const std::string_view& text) {
            const std::array<char, alphabet_size> sorted_alphabet { extract_sorted_alphabet(text) };
            // split test line-by-line
            auto lines { split_lines_and_words(text) };
            auto lines_it { lines.begin() };
            // pull out header and split on space
            auto header_words { *lines_it };
            ++lines_it;
            // pull out rows
            std::array<WEIGHT, alphabet_size * alphabet_size> ret {};
            while (lines_it != lines.end()) {
                auto row_words { *lines_it };
                ++lines_it;
                // split row
                auto row_words_it { row_words.begin() };
                // pull right value
                auto down_elem_view { *row_words_it };
                char down_elem { extract_symbol(down_elem_view) };
                ++row_words_it;
                auto header_words_split_it { header_words.begin() };
                while (header_words_split_it != header_words.end()) {
                    // pull down value
                    auto right_elem_view { *header_words_split_it };
                    char right_elem { extract_symbol(right_elem_view) };
                    ++header_words_split_it;
                    // pull weight
                    auto weight_str { *row_words_it };
                    ++row_words_it;
                    WEIGHT weight {};
                    bool convert_success { std::istringstream { weight_str.data() } >> weight };
                    if constexpr (debug_mode) {
                        if (!convert_success) {
                            throw std::runtime_error { "Failed to convert string to numeric" };
                        }
                    }
                    // insert weight
                    std::size_t ret_idx { to_weights_idx(sorted_alphabet, down_elem, right_elem) };
                    ret[ret_idx] = weight;
                }
            }
            return ret;
        }

        const std::array<char, alphabet_size> alphabet;
        const std::array<WEIGHT, alphabet_size * alphabet_size> weights;

    public:
        /**
         * Construct an @ref
         * offbynull::aligner::scorers::single_character_substitution_matrix_scorer::single_character_substitution_matrix_scorer
         * instance from a text table. A text table is a space-separated table where ...
         *
         *  * the first row contains ASCII characters, representing the first element being compared.
         *  * the first columns contains ASCII characters, representing the second element being compared.
         *  * each cell (not including first row and column) is a score for the two characters corresponding to the cell's position within
         *    the first row and first column.
         *
         * For example, the following text table scores `(A,C)` as -1.
         *
         * ```
         *    A  C  T  G
         * A  1 -1 -1 -1
         * C -1  1 -1  0
         * T -1 -1  1 -1
         * G -1  0 -1  1
         * ```
         *
         * @param text_table Text table.
         */
        single_character_substitution_matrix_scorer(
            const std::string_view& text_table
        )
        : alphabet { std::move(extract_sorted_alphabet(text_table)) }
        , weights { std::move(extract_sorted_weights(text_table)) } {}

        /**
         * Construct an @ref
         * offbynull::aligner::scorers::single_character_substitution_matrix_scorer::single_character_substitution_matrix_scorer
         * instance from an array of characters and scores.
         *
         * @param alphabet_ ASCII characters, representing elements being compared. Must be sorted.
         * @param weights_ Scores for a 2-tuple containing an element from `alphabet_` at each position. Must be sorted by the 2-tuple pair
         *     the score is for. For example, if `alphabet_="ABC"`, then the order of scores must be such that the score
         *     for `(A,A)` appears first, followed by `(A,B)`, followed by `(A,C)`, etc... Must be
         *     sized `alphabet_.size()*alphabet_.size()`.
         */
        single_character_substitution_matrix_scorer(
            const std::array<char, alphabet_size>& alphabet_,  // must be sorted
            const std::array<WEIGHT, alphabet_size * alphabet_size>& weights_  // must be sorted (based on alphabet pair)
        )
        : alphabet { alphabet_ }
        , weights { weights_ } {}

        // This must be operator()() - if you do operator(), doxygen won't recognize it. This isn't the case with other functions (if you
        // leave out the parenthesis, doxygen copies the documentation just fine).
        /**
         * @copydoc offbynull::aligner::scorer::scorer::unimplemented_scorer::operator()()
         */
        WEIGHT operator()(
            [[maybe_unused]] const auto& edge,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const char>
                >
            > down_elem,
            const std::optional<
                std::pair<
                    SEQ_INDEX,
                    std::reference_wrapper<const char>
                >
            > right_elem
        ) const {
            if (down_elem.has_value() && right_elem.has_value()) {
                std::size_t weights_idx {
                    to_weights_idx(
                        alphabet,
                        std::get<1zu>(*down_elem).get(),
                        std::get<1zu>(*right_elem).get()
                    )
                };
                return weights[weights_idx];
            }
            if constexpr (std::numeric_limits<WEIGHT>::has_quiet_NaN) {
                return std::numeric_limits<WEIGHT>::quiet_NaN();
            } else {
                return std::numeric_limits<WEIGHT>::max();
            }
        }
    };

    static_assert(
        scorer<
            single_character_substitution_matrix_scorer<true, 95zu, std::size_t, float>,
            std::pair<int, int>,
            std::size_t,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_SINGLE_CHARACTER_SUBSTITUTION_MATRIX_SCORER_H
