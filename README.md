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
* ~~const correct graph implementations~~

* scorer implementations
  * ~~constant scorer~~
  * ~~match/mismatch scorer~~
  * ~~Levenshtein scorer~~
  * ~~ascii substitution matrix scorer~~
  * ~~pam/blosum scorer~~
  * qwerty-keyboard scorer
  * ~~FIX single_character_substitution_matrix_scorer.h -- LAST CHAR IS NOT INDEL SCORE~~

* ~~FIX EXTENDEDGAP slice_nodes() function to not use forward_range_join_view~~
  * ~~make adapater that create forward_range / forward_backward_range based on callbacks (e.g. next(value) func and prev(value) func)~~

* resident_segmenter - some use of std::vector -- move creation of types in container_pack_creators (SEE TODOs IN RESIDENT_SEGMENTER -- DOES IT EVEN NEED TO EXIST?)
* ~~sliceable_pairwise_alignment_graph_backtracker use bidiwalker instead (make sure to update bidiwalker's logic to what's in backtracker)~~
* ~~sliceable_pairwise_alignment_graph_backtracker randomized tests against pairwise_alignment_graph_backtracker~~
* run profiler and optmize functions (some local alignment functions may be doing a ton of unneeded work -- e.g. get_outputs_full)

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