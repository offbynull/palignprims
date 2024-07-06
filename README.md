TODO:
* ~~There's a huge influx of "container_creator" type parameters -- find a better way to package this up?~~
* ~~Typename GRAPH to G~~
* ~~Add explicit concepts for node and edge~~
* ~~Pull out container packs into their own header file~~
* Fix sliceable backtracker to not have to sort, but use grid position + depth for positioning
  * This use to be how it was with pairwise_alignment_graph_backtracker/slot_container.h and IS HOW ITS CURRENTLY BEING
    done in sliceable_pairwise_alignment_graph_backtracker/slot_container.h -- You need to update this so that you both
    have sorting AND depth positioning, and switch between them using a compile-time flag.
* Fix pairwise / pairwise sliceable to NOT take in functor to get edge weight + NOT take in WEIGHT param
* sequence implementations for the following (see seqeunce concept)
  * ~~wrapper that pads out with dummy (or truncates) to a certain size~~ 
  * ~~mmap'd file data~~
  * ~~sliding window over existing view~~
  * ~~zip sequence~~
  * ~~chunk sequence~~
  * ~~iota sequence (integer order of an array, you specify start offset and size)~~
  * ~~transform sequence (transform index to object based on lambda)~~
  * decompressing compressed bytes -- when a location is accessed, decompress that chunk and hold it in cache

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