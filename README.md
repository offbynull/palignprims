TODO:
* ~~Fix include guards to be full path, not just filename~~
* Change all occurrences of size_t to std::size_t
* ~~Use std::unreachable() any place with known unreachable?~~
* ~~Remove underscores from template parameters because they're causing warnings.~~
* Add function for stack 8x8, 16x16, etc..
* Add concept checks to autos
* Add random access container wrapper that pads out with dummy (or truncates) to a certain size
* Add random access container wrapper that mmap
* Add random access container wrapper that decompresses as you read
* Add divide-and-conquer aligner interface methods and algorithm

  ```c++
  auto slice_nodes(INDEX column); //
  class pairwise_reversed_graph;  // graph reverses a pairwise graph's edges, so that leaf is root and vice versa.
  class pairwise_graph_view;      // graph that limits the down/right node counts
  ```
  
* Integer promotion rules hit when you use types smaller than int:
  * e.g. subtracting two chars results in an int, meaning if you have char x = c1 - c2, you'll get "warning: conversion from ‘int’ to ‘unsigned char’ may change value [-Warith-conversion]"
  * Lots of places +1u -1u or == 0u, use offbynull::utils::constants<T> to get literals of the correct type ()
  * option 1: you might have to forcefully static_cast every addition / subtraction?
  * option 2: you might have to write a integer wrapper that avoids integer promotion rules (internally static_casts)? 