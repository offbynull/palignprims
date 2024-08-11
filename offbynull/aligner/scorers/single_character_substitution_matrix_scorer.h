#ifndef OFFBYNULL_ALIGNER_SCORERS_PRINTABLE_SINGLE_CHARACTER_SUBSTITUTION_MATRIX_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_PRINTABLE_SINGLE_CHARACTER_SUBSTITUTION_MATRIX_SCORER_H

#include <optional>
#include <array>
#include <algorithm>
#include <sstream>
#include <limits>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/utils.h"

namespace offbynull::aligner::scorers::single_character_substitution_matrix_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::utils::type_displayer;

    template<bool debug_mode, weight WEIGHT, std::size_t ALPHABET_SIZE>
    class single_character_substitution_matrix_scorer {
    private:
        static_assert(ALPHABET_SIZE <= 255zu, "Alphabet greater than 255 symbols");

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
            const char* start_char_ptr { &start_char_ref }; // can't use line.begin() directly because line is an iterator object, not a pointer -- need to first grab the object from the iterator (which is a ref into a element in text) then get the pointer to it
            return std::string_view { start_char_ptr, std::ranges::distance(trailing_trimmed) };
        }

        static auto split_lines_and_words(const auto& text) {
            auto text_normalized_ends {
                text
                | std::views::filter([](const auto& ch) { return ch != '\r'; })
            };
            return
                std::views::split(text_normalized_ends, '\n')
                | std::views::transform([](const auto& line) {
                    static_assert(std::is_same_v<decltype(*line.begin()), const char&>); // ensure you're getting back a REFERENCE to the original data when you access
                    const char& start_char_ref { *line.begin() };
                    const char* start_char_ptr { &start_char_ref }; // can't use line.begin() directly because line is an iterator object, not a pointer -- need to first grab the object from the iterator (which is a ref into a element in text) then get the pointer to it
                    return std::string_view { start_char_ptr, std::ranges::distance(line) };
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
                        | std::ranges::views::filter([](const auto& s) {
                            return s.begin() != s.end();
                        });
                });
        }

        static char extract_symbol(const auto& element_view) {
            auto element_view_it { element_view.begin() };
            if constexpr (debug_mode) {
                if (element_view_it == element_view.end()) {
                    throw std::runtime_error("Expected 1 character for each alphabet item -- empty");
                }
            }
            char ret { *element_view_it };
            if constexpr (debug_mode) {
                ++element_view_it;
                if (element_view_it != element_view.end()) {
                    throw std::runtime_error("Expected 1 character for each alphabet item -- multiple");
                }
            }
            return ret;
        }

        static std::array<char, ALPHABET_SIZE> extract_sorted_alphabet(const std::string_view& text) {
            // split test line-by-line
            auto lines { split_lines_and_words(text) };
            auto lines_it { lines.begin() };
            // pull out header and split on space
            auto header_words { *lines_it };
            // set alphabet
            if constexpr (debug_mode) {
                auto size { std::ranges::distance(header_words) };
                if (size != ALPHABET_SIZE) {
                    throw std::runtime_error("Unexpected number of characters");
                }
            }
            std::array<char, ALPHABET_SIZE> ret {};
            auto header_words_it { header_words.begin() };
            for (std::size_t i { 0zu }; i < ALPHABET_SIZE; i++) {
                ret[i] = extract_symbol(*header_words_it);
                ++header_words_it;
            }
            std::ranges::sort(ret.begin(), ret.end());
            return ret;
        }

        static std::size_t to_weights_idx(
            const std::array<char, ALPHABET_SIZE> sorted_alphabet,
            const char down_elem,
            const char right_elem
        ) {
            auto down_it { std::lower_bound(sorted_alphabet.begin(), sorted_alphabet.end(), down_elem) };
            if constexpr (debug_mode) {
                if (*down_it != down_elem) {
                    throw std::runtime_error("Not found");
                }
            }
            auto right_it { std::lower_bound(sorted_alphabet.begin(), sorted_alphabet.end(), right_elem) };
            if constexpr (debug_mode) {
                if (*right_it != right_elem) {
                    throw std::runtime_error("Not found");
                }
            }
            std::size_t down_idx { down_it - sorted_alphabet.begin() };
            std::size_t right_idx { right_it - sorted_alphabet.begin() };
            return down_idx * ALPHABET_SIZE + right_idx;
        }

        static std::array<WEIGHT, ALPHABET_SIZE * ALPHABET_SIZE> extract_sorted_weights(const std::string_view& text) {
            const std::array<char, ALPHABET_SIZE> sorted_alphabet { std::move(extract_sorted_alphabet(text)) };
            // split test line-by-line
            auto lines { split_lines_and_words(text) };
            auto lines_it { lines.begin() };
            // pull out header and split on space
            auto header_words { *lines_it };
            ++lines_it;
            // pull out rows
            std::array<WEIGHT, ALPHABET_SIZE * ALPHABET_SIZE> ret {};
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
                            throw std::runtime_error("Failed to convert string to numeric");
                        }
                    }
                    // insert weight
                    std::size_t ret_idx { to_weights_idx(sorted_alphabet, down_elem, right_elem) };
                    ret[ret_idx] = weight;
                }
            }
            return ret;
        }

        const std::array<char, ALPHABET_SIZE> alphabet;
        const std::array<WEIGHT, ALPHABET_SIZE * ALPHABET_SIZE> weights;

    public:
        single_character_substitution_matrix_scorer(
            const std::string_view& text_table
        )
        : alphabet { std::move(extract_sorted_alphabet(text_table)) }
        , weights { std::move(extract_sorted_weights(text_table)) } {}

        single_character_substitution_matrix_scorer(
            const std::string_view& text_table,
            const char
        )
        : alphabet { std::move(extract_sorted_alphabet(text_table)) }
        , weights { std::move(extract_sorted_weights(text_table)) } {}

        WEIGHT operator()(
            const auto& edge,
            const std::optional<std::reference_wrapper<const char>> down_elem,
            const std::optional<std::reference_wrapper<const char>> right_elem
        ) const {
            if (down_elem.has_value() && right_elem.has_value()) {
                std::size_t weights_idx { to_weights_idx(alphabet, down_elem.value().get(), right_elem.value().get()) };
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
            single_character_substitution_matrix_scorer<true, float, 95zu>,
            std::pair<int, int>,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_PRINTABLE_SINGLE_CHARACTER_SUBSTITUTION_MATRIX_SCORER_H
