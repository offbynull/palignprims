TODO:
* ~~There's a huge influx of "container_creator" type parameters -- find a better way to package this up?~~
* ~~Typename GRAPH to G~~
* ~~Add explicit concepts for node and edge~~
* ~~Pull out container packs into their own header file~~
* ~~Fix sliceable backtracker to not have to sort, but use grid position + depth for positioning~~
* ~~Fix pairwise / pairwise sliceable to NOT take in functor to get edge weight + NOT take in WEIGHT param~~
* sequence implementations for the following (see seqeunce concept)
  * ~~wrapper that pads out with dummy (or truncates) to a certain size~~ 
  * ~~mmap'd file data~~
  * ~~sliding window over existing view~~
  * ~~zip sequence~~
  * ~~chunk sequence~~
  * ~~iota sequence (integer order of an array, you specify start offset and size)~~
  * ~~transform sequence (transform index to object based on lambda)~~
  * decompressing compressed bytes -- when a location is accessed, decompress that chunk and hold it in cache
  * alphabet bitpacked (e.g. ACTG only 2 bits per element)
* ~~const correct graph implementations~~

* scorer implementations
  * ~~constant scorer~~
  * ~~match/mismatch scorer~~
  * ~~Levenshtein scorer~~
  * ~~ascii substitution matrix scorer~~
  * ~~pam/blosum scorer~~
  * ~~FIX single_character_substitution_matrix_scorer.h -- LAST CHAR IS NOT INDEL SCORE~~
  * ~~qwerty-keyboard scorer~~

* ~~FIX EXTENDEDGAP slice_nodes() function to not use forward_range_join_view~~
  * ~~make adapater that create forward_range / forward_backward_range based on callbacks (e.g. next(value) func and prev(value) func)~~

* ~~revise container_pack pattern being used across project to how it's being done in sliceable_pairwise_alignment_graph_backtracker~~ 
* ~~graph node/edge types to structs instead of std::pair -- add friend functions that print out to std::out~~
* ~~graph node/edge types -- add concepts to ensure they're regular types BUT NOT < operator (backtrakcers that need < operator have concept checks for them)~~
* ~~graphviz dump -- use string formatter / std::cout on node and edge outputs~~
* ~~local/fitting/overlap graph -- clean up ranges being produced so it doesn't run a filter function over an unnecessary number of nodes~~
* ~~error_check -> debug_mode~~
* ~~remove limits() function -- move fields directly into graph class and make them constexpr~~
  * ~~at least move grid depth into graph class, making it constexpr~~
  * ~~move max_path_edge_cnt~~
  * ~~move max_resident_nodes_cnt~~
  * ~~remove limits() function entirely~~
* ~~container creators -- make them into functions so you don't have to create an instance of container_creator each time you use it~~
* container creator packs -- vectors being created can be reserved to max size???? MAKE THIS AN OPTIONAL PARAMETER IN THE CONTAINER CREATOR PACK OR THE FUNCTION BEING CALLED?
* ~~max_path_edge_cnt/max_resident_nodes_cnt to path_edge_capacity/resident_nodes_capacity~~
* ~~const correct container_creator_packs~~
* ~~CHANGE const G& graph TO const G& g~~
* sliced_subdivider -- changed "whole_graph" to "graph_partition"?
* add E_COUNT type to readable_pairwise_alignment_graph concept and graph implementations -- make use of it in slot_container.h rather than asking the user to supply it 
  * E_COUNT is a type wide enough to hold the maximum number of edges from one node to another 
  * or ... use "offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/utils.h" to determine what the correct type should be
* ~~max_grid_node_depth should be a parameter directly on the class, EVALUATABLE AT COMPILE_TIME~~
* ~~resident_segmenter - some use of std::vector -- move creation of types in container_pack_creators (SEE TODOs IN RESIDENT_SEGMENTER -- DOES IT EVEN NEED TO EXIST?)~~
* ~~resident_segmenter - move is_node_on_max_path/walk_to_node to static functions in bidiwalker?~~
* DONT DO THIS, but make a note that it's possible to do in the code (but may result in rounding errors):
  * make it so forward_walker can move backward, so if you're at slice n, you can subtract your way back to slice n-1
  * make it so backward_walker can move backward, so if you're at slice n, you can subtract your way back to slice n+1
  * make it so bidiwalker can move at will (making use of both points above)
* ~~resident_segmenter, when finding if a resident node is a partition point, should first get the final weight of the graph, then TEST the bidiwalk'd weight at that node to see if it matches the final weight (USER MUST DEFINE TOLERANCE)~~
* ~~subdivider, when finding the node in the slice that's being passed through, should first get the final weight of the graph, then TEST the bidiwalk'd weight at that node to see if it matches the final weight (USER MUST DEFINE TOLERANCE)~~ DONT NEED TO DO THIS the assumption with subdivider is that at least 1 node in each slice goes over max path, so it's fine just pulling out the max weight for a slice
* ~~sliceable_pairwise_alignment_graph_backtracker use bidiwalker instead (make sure to update bidiwalker's logic to what's in backtracker)~~
* ~~sliceable_pairwise_alignment_graph_backtracker randomized tests against pairwise_alignment_graph_backtracker~~
* ~~graph constructors take in scorers using std::function<...> -- uses scorer concept instead? e.g. scorer auto&& indel_scorer, scorer&& substitution scorer, etc...~~
* ~~remove all std::function usages~~
* move out all "to_vector" functions as utility func
* concepts should enforce that type being enforced is std::same_as<std::decay_t<T>, T>, meaning no cv and no refs -- maybe use std::remove_cvref instead of std::decay?
  * template<typename T>
    concept decayable_type = !std::is_void_v<T> && std::is_convertible_v<T, std::decay_t<T>>;
* create factory functions for everything that has heavy templating
* ~~remove pointers in forward_walker and path_container -- requires custom move/copy constructors + custom assignment operators~~ (leaving this as-is for now, with the custom constructors and assignment op overrides)

* doxygen documentation
* run profiler and optimize functions (some local alignment functions may be doing a ton of unneeded work -- e.g. get_outputs_full)
* ~~fix compiler warnings~~
* cleanup syntax
  * some identifiers starting with _, change so they end with _ 
  * ~~140char lines~~
  * ~~spaces between squiggly brackets: {}~~
  * python helper to ensure ...
    * ~~140 char line~~ 
    * ~~guards are correct~~
    * ~~#includes are there for all std:: and boost::~~
    * #includes are there for all offbynull::

* multithreaded backtracker -- should be easy

* Update get_in_degree() / get_out_degree() functions to calculate directly
* Add concept checks to autos
* ~~Add random access container wrapper that mmap (it's in boost)~~
* ~~Add random access container wrapper that decompresses as you read~~
* ~~Add divide-and-conquer aligner interface methods and algorithm~~

* Integer promotion rules hit when you use types smaller than int:
  * e.g. subtracting two chars results in an int, meaning if you have char x = c1 - c2, you'll get "warning: conversion from ‘int’ to ‘unsigned char’ may change value [-Warith-conversion]"
  * Lots of places +1u -1u or == 0u, use offbynull::utils::constants<T> to get literals of the correct type ()
  * option 1: you might have to forcefully static_cast every addition / subtraction?
  * option 2: you might have to write a integer wrapper that avoids integer promotion rules (internally static_casts)? 