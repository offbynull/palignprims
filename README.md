TODO:
* ~~Fix include guards to be full path, not just filename~~
* Change all occurrences of size_t to std::size_t
* ~~Use std::unreachable() any place with known unreachable?~~
* ~~Remove underscores from template parameters because they're causing warnings.~~
* ~~Add function for stack 8x8, 16x16, etc..~~
* Change references of input_iterator to forward_iterator
  * When you change this, you may be able to swap out some of your custom implementations of stuff (in utils.h) to standard stl funcs
* 
* Update get_in_degree() / get_out_degree() functions to calculate directly
* Add concept checks to autos
* Add random access container wrapper that pads out with dummy (or truncates) to a certain size
* Add random access container wrapper that mmap (it's in boost)
* Add random access container wrapper that decompresses as you read
* Add divide-and-conquer aligner interface methods and algorithm

  ```c++
  constexpr std::size_t G::max_slice_predecessor_count; // make number of predecessors required by a slice
  auto slice_walk(INDEX column); // returns a slice in order of dependencies
  auto slice_contants();  // returns node weights that MUST be always be kept in-memory (in addition to slice_walk(i-1)
  
  class pairwise_reversed_graph;  // graph reverses a pairwise graph's edges, so that leaf is root and vice versa.
  class pairwise_graph_view;      // graph that limits the down/right node counts
  ```
  
* Integer promotion rules hit when you use types smaller than int:
  * e.g. subtracting two chars results in an int, meaning if you have char x = c1 - c2, you'll get "warning: conversion from ‘int’ to ‘unsigned char’ may change value [-Warith-conversion]"
  * Lots of places +1u -1u or == 0u, use offbynull::utils::constants<T> to get literals of the correct type ()
  * option 1: you might have to forcefully static_cast every addition / subtraction?
  * option 2: you might have to write a integer wrapper that avoids integer promotion rules (internally static_casts)? 