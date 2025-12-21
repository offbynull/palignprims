TODO:

* update meson based on new cookiecutter templates.

* enable struct packing via  a template parameter instead of a define? may cause lots of duplication. 

* sequence implementations for the following (see seqeunce concept)
  * decompressing compressed bytes -- when a location is accessed, decompress that chunk and hold it in cache
  * alphabet bitpacked (e.g. ACTG only 2 bits per element)

* sliced_subdivider -- changed "whole_graph" to "graph_partition"?
 
* DONT DO THIS, but make a note that it's possible to do in the code (but may result in rounding errors):
  * make it so forward_walker can move backward, so if you're at slice n, you can subtract your way back to slice n-1
  * make it so backward_walker can move backward, so if you're at slice n, you can subtract your way back to slice n+1
  * make it so bidiwalker can move at will (making use of both points above)

* run profiler and optimize functions (some local alignment functions may be doing a ton of unneeded work -- e.g. get_outputs_full)
  * did this but didn't bother going too deep into the flame graph -- make sure to turn off D_GLIBCXX_DEBUG to get true results

* multithreaded backtracker -- should be easy
  * option 1: multithreaded pairwise_alignment_graph_backtracker that breaks up the graph into chunks and process each chunk in a different thread (taking dependency order of the chunks into account)
  * option 2: singlethreaded sliceable_pairwise_alignment_graph_backtracker finds points in the graph at regular intervals, then multithreaded pairwise_alignment_graph_backtracker fills in between those intervals
  * ~~generic_row_nodes()~~
  * ~~generic_diagonal_nodes_segmented() CONTINUE TESTING~~
  * UPDATE GRAPH IMPLEMENTATIONS:
    * row_nodes() grid_down param should be relative to root_node parameter, not graph's actual root node
    * ~~segmented_diagonal_node's grid_axis_position parma should be relative to root_node parameter, not graph's actual root node~~ 
  * CONTINUE IMPLEMENTING DIAGONAL_FORWARDWALKER ONCE generic_diagonal_nodes_segmented() READY

* Update get_in_degree() / get_out_degree() functions to calculate directly
* Add concept checks to autos

* Integer promotion rules hit when you use types smaller than int:
  * e.g. subtracting two chars results in an int, meaning if you have char x = c1 - c2, you'll get "warning: conversion from ‘int’ to ‘unsigned char’ may change value [-Warith-conversion]"
  * Lots of places +1u -1u or == 0u, use offbynull::utils::constants<T> to get literals of the correct type ()
  * option 1: you might have to forcefully static_cast every addition / subtraction?
  * option 2: you might have to write a integer wrapper that avoids integer promotion rules (internally static_casts)? 