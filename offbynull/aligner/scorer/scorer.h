#ifndef OFFBYNULL_ALIGNER_SCORER_SCORER_H
#define OFFBYNULL_ALIGNER_SCORER_SCORER_H

#include <concepts>
#include <optional>
#include <functional>
#include <type_traits>
#include "offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h"
#include "offbynull/aligner/concepts.h"
#include "offbynull/concepts.h"

namespace offbynull::aligner::scorer::scorer {
    using offbynull::aligner::concepts::weight;
    using offbynull::aligner::graph::sliceable_pairwise_alignment_graph::readable_sliceable_pairwise_alignment_graph;
    using offbynull::concepts::unqualified_value_type;

    template<typename T, typename EDGE, typename DOWN_ELEM, typename RIGHT_ELEM, typename WEIGHT>
    concept scorer =
        unqualified_value_type<T>
        && requires(
            T t,
            const EDGE& edge,
            const std::optional<std::reference_wrapper<const DOWN_ELEM>> down_elem_ref_opt,
            const std::optional<std::reference_wrapper<const RIGHT_ELEM>> right_elem_ref_opt
        ) {
            { t(edge, down_elem_ref_opt, right_elem_ref_opt) } -> std::same_as<WEIGHT>;
        }
        && std::is_same_v<std::remove_cvref_t<DOWN_ELEM>, DOWN_ELEM>  // make sure no refs / cv-qualifiers
        && std::is_same_v<std::remove_cvref_t<RIGHT_ELEM>, RIGHT_ELEM>  // make sure no refs / cv-qualifiers
        && weight<WEIGHT>;
}

#endif //OFFBYNULL_ALIGNER_SCORER_SCORER_H
