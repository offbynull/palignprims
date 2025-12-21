#ifndef OFFBYNULL_ALIGNER_SCORERS_QWERTY_SCORER_H
#define OFFBYNULL_ALIGNER_SCORERS_QWERTY_SCORER_H

#include <cstddef>
#include <ranges>
#include <array>
#include <algorithm>
#include <utility>
#include <variant>
#include <tuple>
#include <cmath>
#include "offbynull/aligner/scorer/scorer.h"
#include "offbynull/aligner/scorers/single_character_substitution_matrix_scorer.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::scorers::qwerty_scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::scorer::scorer::scorer;
    using offbynull::aligner::scorers::single_character_substitution_matrix_scorer::single_character_substitution_matrix_scorer;
    using offbynull::concepts::widenable_to_size_t;

    /** 2D point. */
    struct point {
        double x;
        double y;
    };

    /** 2D horizontal line. */
    struct horizontal_line {
        double x_left;
        double x_right;
        double y;
    };

    /**
     * @ref offbynull::aligner::scorer::scorer::scorer which scores printable ASCII characters via their distance from each other on a
     * typical QWERTY-layout keyboard.
     *
     * @tparam debug_mode `true` to enable debugging logic, `false` otherwise.
     * @tparam SEQ_INDEX Sequence indexer type.
     * @tparam WEIGHT Alignment graph's edge weight type.
     */
    template<bool debug_mode, widenable_to_size_t SEQ_INDEX, weight WEIGHT>
    class qwerty_scorer : public single_character_substitution_matrix_scorer<debug_mode, 95zu, SEQ_INDEX, WEIGHT> {
    public:
        /**
         * Construct an @ref offbynull::aligner::scorers::qwerty_scorer::qwerty_scorer instance.
         */
        // Why not force computing create_qwerty_distance_matrix() at compile-time via consteval? It seems g++ (and other compilers) limit
        // the amount of work that can happen during compile-time evaluation. Too much work results in ...
        //
        //     evaluation operation count exceeds limit of 33554432 (use ‘-fconstexpr-ops-limit=’ to increase the limit)
        //
        // As such, this computation is being performed at run-time.
        //
        // TODO: Force evaluation offline and directly embed the resulting array rather than relying on consteval?
        qwerty_scorer()
        : qwerty_scorer { create_qwerty_distance_matrix() } {}

    private:
        qwerty_scorer(const std::pair<std::array<char, 95zu>, std::array<WEIGHT, 95zu*95zu>>& sorted_alphabet_and_weights)
        : single_character_substitution_matrix_scorer<debug_mode, 95zu, SEQ_INDEX, WEIGHT> {
            std::get<0zu>(sorted_alphabet_and_weights),
            std::get<1zu>(sorted_alphabet_and_weights)
        } {}

        static constexpr auto distance_(const point& p1, const point& p2) {
            return std::hypot(p2.x - p1.x, p2.y - p1.y);
        }

        static constexpr auto distance_(const point& p1, const horizontal_line& l2) {
            if (p1.x < l2.x_left) {
                return distance_(p1, point { l2.x_left, l2.y });
            } else if (p1.x > l2.x_right) {
                return distance_(p1, point { l2.x_right, l2.y });
            } else {
                return distance_(p1, point { p1.x, l2.y });
            }
            std::unreachable();
        }

        static constexpr auto distance_(const horizontal_line& l1, const horizontal_line& l2) {
            if (l1.x_left >= l2.x_left && l1.x_left <= l2.x_right) {
                return std::abs(l2.y - l1.y);
            } else if (l1.x_right >= l2.x_left && l1.x_right <= l2.x_right) {
                return std::abs(l2.y - l1.y);
            } else if (l1.x_right < l2.x_left) {
                return distance_(point { l1.x_right, l1.y }, point { l2.x_left, l2.y });
            } else if (l1.x_left > l2.x_right) {
                return distance_(point { l1.x_left, l1.y }, point { l2.x_right, l2.y });
            }
            std::unreachable();
        }

        static constexpr auto distance(
            const std::variant<point, horizontal_line>& item1,
            const std::variant<point, horizontal_line>& item2
        ) {
            if (std::holds_alternative<point>(item1) && std::holds_alternative<point>(item2)) {
                const auto& p1 { std::get<point>(item1) };
                const auto& p2 { std::get<point>(item2) };
                return distance_(p1, p2);
            } else if (std::holds_alternative<horizontal_line>(item1) && std::holds_alternative<point>(item2)) {
                const auto& l1 { std::get<horizontal_line>(item1) };
                const auto& p2 { std::get<point>(item2) };
                return distance_(p2, l1);
            } else if (std::holds_alternative<point>(item1) && std::holds_alternative<horizontal_line>(item2)) {
                const auto& p1 { std::get<point>(item1) };
                const auto& l2 { std::get<horizontal_line>(item2) };
                return distance_(p1, l2);
            } else if (std::holds_alternative<horizontal_line>(item1) && std::holds_alternative<horizontal_line>(item2)) {
                const auto& l1 { std::get<horizontal_line>(item1) };
                const auto& l2 { std::get<horizontal_line>(item2) };
                return distance_(l1, l2);
            }
            std::unreachable();
        }


        static constexpr std::pair<std::array<char, 95zu>, std::array<WEIGHT, 95zu*95zu>> create_qwerty_distance_matrix(
            WEIGHT dist_offset = static_cast<WEIGHT>(1.9)
        ) {
            struct entry {
                char key;
                std::variant<point, horizontal_line> position;
            };

            std::array<entry, 95zu> key_positions {
                entry { '`', point { 0.0, 0.0 } }, entry { '~', point { 0.0, 0.0 } },
                entry { '1', point { 1.0, 0.0 } }, entry { '!', point { 1.0, 0.0 } },
                entry { '2', point { 2.0, 0.0 } }, entry { '@', point { 2.0, 0.0 } },
                entry { '3', point { 3.0, 0.0 } }, entry { '#', point { 3.0, 0.0 } },
                entry { '4', point { 4.0, 0.0 } }, entry { '$', point { 4.0, 0.0 } },
                entry { '5', point { 5.0, 0.0 } }, entry { '%', point { 5.0, 0.0 } },
                entry { '6', point { 6.0, 0.0 } }, entry { '^', point { 6.0, 0.0 } },
                entry { '7', point { 7.0, 0.0 } }, entry { '&', point { 7.0, 0.0 } },
                entry { '8', point { 8.0, 0.0 } }, entry { '*', point { 8.0, 0.0 } },
                entry { '9', point { 9.0, 0.0 } }, entry { '(', point { 9.0, 0.0 } },
                entry { '0', point { 10.0, 0.0 } }, entry { ')', point { 10.0, 0.0 } },
                entry { '-', point { 11.0, 0.0 } }, entry { '_', point { 11.0, 0.0 } },
                entry { '=', point { 12.0, 0.0 } }, entry { '+', point { 12.0, 0.0 } },
                entry { 'q', point { 1.0+0.333, 1.0 } }, entry { 'Q', point { 1.0+0.333, 1.0 } },
                entry { 'w', point { 2.0+0.333, 1.0 } }, entry { 'W', point { 2.0+0.333, 1.0 } },
                entry { 'e', point { 3.0+0.333, 1.0 } }, entry { 'E', point { 3.0+0.333, 1.0 } },
                entry { 'r', point { 4.0+0.333, 1.0 } }, entry { 'R', point { 4.0+0.333, 1.0 } },
                entry { 't', point { 5.0+0.333, 1.0 } }, entry { 'T', point { 5.0+0.333, 1.0 } },
                entry { 'y', point { 6.0+0.333, 1.0 } }, entry { 'Y', point { 6.0+0.333, 1.0 } },
                entry { 'u', point { 7.0+0.333, 1.0 } }, entry { 'U', point { 7.0+0.333, 1.0 } },
                entry { 'i', point { 8.0+0.333, 1.0 } }, entry { 'I', point { 8.0+0.333, 1.0 } },
                entry { 'o', point { 9.0+0.333, 1.0 } }, entry { 'O', point { 9.0+0.333, 1.0 } },
                entry { 'p', point { 10.0+0.333, 1.0 } }, entry { 'P', point { 10.0+0.333, 1.0 } },
                entry { '[', point { 11.0+0.333, 1.0 } }, entry { '{', point { 11.0+0.333, 1.0 } },
                entry { ']', point { 12.0+0.333, 1.0 } }, entry { '}', point { 12.0+0.333, 1.0 } },
                entry { '\\', point { 13.0+0.333, 1.0 } }, entry { '|', point { 13.0+0.333, 1.0 } },
                entry { 'a', point { 1.0+0.666, 2.0 } }, entry { 'A', point { 1.0+0.666, 2.0 } },
                entry { 's', point { 2.0+0.666, 2.0 } }, entry { 'S', point { 2.0+0.666, 2.0 } },
                entry { 'd', point { 3.0+0.666, 2.0 } }, entry { 'D', point { 3.0+0.666, 2.0 } },
                entry { 'f', point { 4.0+0.666, 2.0 } }, entry { 'F', point { 4.0+0.666, 2.0 } },
                entry { 'g', point { 5.0+0.666, 2.0 } }, entry { 'G', point { 5.0+0.666, 2.0 } },
                entry { 'h', point { 6.0+0.666, 2.0 } }, entry { 'H', point { 6.0+0.666, 2.0 } },
                entry { 'j', point { 7.0+0.666, 2.0 } }, entry { 'J', point { 7.0+0.666, 2.0 } },
                entry { 'k', point { 8.0+0.666, 2.0 } }, entry { 'K', point { 8.0+0.666, 2.0 } },
                entry { 'l', point { 9.0+0.666, 2.0 } }, entry { 'L', point { 9.0+0.666, 2.0 } },
                entry { ';', point { 10.0+0.666, 2.0 } }, entry { ':', point { 10.0+0.666, 2.0 } },
                entry { '\'', point { 11.0+0.666, 2.0 } }, entry { '"', point { 11.0+0.666, 2.0 } },
                entry { 'z', point { 1.0+1.000, 3.0 } }, entry { 'Z', point { 1.0+1.000, 3.0 } },
                entry { 'x', point { 2.0+1.000, 3.0 } }, entry { 'X', point { 2.0+1.000, 3.0 } },
                entry { 'c', point { 3.0+1.000, 3.0 } }, entry { 'C', point { 3.0+1.000, 3.0 } },
                entry { 'v', point { 4.0+1.000, 3.0 } }, entry { 'V', point { 4.0+1.000, 3.0 } },
                entry { 'b', point { 5.0+1.000, 3.0 } }, entry { 'B', point { 5.0+1.000, 3.0 } },
                entry { 'n', point { 6.0+1.000, 3.0 } }, entry { 'N', point { 6.0+1.000, 3.0 } },
                entry { 'm', point { 7.0+1.000, 3.0 } }, entry { 'M', point { 7.0+1.000, 3.0 } },
                entry { ',', point { 8.0+1.000, 3.0 } }, entry { '<', point { 8.0+1.000, 3.0 } },
                entry { '.', point { 9.0+1.000, 3.0 } }, entry { '>', point { 9.0+1.000, 3.0 } },
                entry { '/', point { 10.0+1.000, 3.0 } }, entry { '?', point { 10.0+1.000, 3.0 } },
                entry { ' ', horizontal_line { 3.0, 9.0, 4.0 } }  // from x { 2.0+1.000, 4.0 } to , { 8.0+1.000, 4.0 }
            };
            // construct alphabet, sorted
            std::array<char, 95zu> alphabet_sorted {};
            for (const auto& [idx, key_pos] : std::views::enumerate(key_positions)) {
                alphabet_sorted[static_cast<std::size_t>(idx)] = key_pos.key;
            }
            std::ranges::sort(alphabet_sorted);
            // construct distances, sorted by alphabet
            std::array<WEIGHT, 95zu*95zu> weights_sorted {};
            for (const auto& key1_pos : key_positions) {
                auto key1_it { std::lower_bound(alphabet_sorted.begin(), alphabet_sorted.end(), key1_pos.key) };
                auto key1_idx { key1_it - alphabet_sorted.begin() };
                for (const auto& key2_pos : key_positions) {
                    auto key2_it { std::lower_bound(alphabet_sorted.begin(), alphabet_sorted.end(), key2_pos.key) };
                    auto key2_idx { key2_it - alphabet_sorted.begin() };
                    auto idx { key1_idx * 95zu + key2_idx };
                    WEIGHT dist {
                        static_cast<WEIGHT>(distance(key1_pos.position, key2_pos.position))
                    };
                    dist = -dist; // negate because farther away should mean lower score, not higher (at this point highest score will be 0)
                    dist += dist_offset; // shift up by some offset (e.g. 1.9), meaning super close substitutions increase the score (at
                                         // this point highest score will be 0+offset)
                    weights_sorted[idx] = dist;
                }
            }
            // return
            return { alphabet_sorted, weights_sorted };
        }
    };


    static_assert(
        scorer<
            qwerty_scorer<true, std::size_t, float>,
            std::pair<int, int>,
            std::size_t,
            char,
            char,
            float
        >
    );
}

#endif //OFFBYNULL_ALIGNER_SCORERS_QWERTY_SCORER_H
