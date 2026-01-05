<p align="center"><img src="palignprims.png" alt="Project logo"></p>

# Pairwise Alignment Primitives

The Pairwise Alignment Primitives project (PAlignPrims) is a C++ header-only library that provides the building blocks for assembling [pairwise](https://en.wiktionary.org/wiki/pairwise) [sequence aligners](https://en.wikipedia.org/wiki/Sequence_alignment). PAlignPrims generalizes alignment beyond bioinformatics, extending it to other domains while supporting broader bioinformatics use cases.

 * <details><summary>Modern design.</summary>
 
   PAlignPrims adopts modern infrastructure and design choices:

   * C++23 standard.
   * Meson build system.
   * Compile-time polymorphism.
   * Compile-time constraint checking.
   * Runtime constraint checking (guardrails to prevent footguns).
   * Documentation styled after high-level languages (e.g., doxygen comment structure mimics Javadocs / Python docstrings).
   * Hierarchical structuring styled after high-level languages (e.g., namespaces mimic Java / Python package naming standards).
   * Import patterns styled after high-level languages (e.g., mimics importing modules in Java / Python).

   </details>
   
 * <details><summary>Efficient design.</summary>
 
   PAlignPrims design choices target efficiency and customizability, providing several knobs to get the best performance for the specific use case and target platform:
   
   * <details><summary>No runtime polymorphism.</summary>
   
     PAlignPrims avoids the use of virtual functions, removing unnecessary indirection to get faster performance.
     
     </details>
     
   * <details><summary>Optional safety.</summary>
   
     PAlignPrims provides several guardrails to prevent common misuse. These guardrails can be disabled (`debug_mode` template parameter), enabling the compiler to make much better optimization choices.
     
     </details>

   * <details><summary>Optimal memory strategy.</summary>
   
     PAlignPrims classes are heavily templated, allowing users to ...
     
     * select types best for their specific use case.
     * select a memory allocation strategy that's best for their specific use case (heap vs stack).
     * optionally pack structs, if it's deemed beneficial for their specific use case / platform.
     
     For example, if an alignment will only be performed on small sequences, the alignment algorithm can perform its operations using narrower integer types and exclusively use the stack. The narrower integer types will save memory and stack allocation will save potentially expensive heap allocation overhead.
     
     </details>

   </details>
   
 * <details><summary>Generic sequences.</summary>
 
   PAlignPrims's [sequence interface](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequence/sequence.h) is flexible enough to hold any element type. For example, a PAlignPrims sequence can hold ...
   
   * characters (e.g., DNA bases, amino acids).
   * musical notes.
   * sensor readings (e.g., weather samples).
   * waveform features (e.g., peaks and frequencies).
   * log events (elements are events, not text).
   * network packets (elements are packets, not bytes).
   * a combination of multiple element types (e.g., musical note paired with sensor readings).
   
   As long as it's a finite sequence, PAlignPrims can represent it.
   
   In addition, PAlignPrims comes bundled with sequence adapters and implementations, including ...
   
   * [chunk sequence](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/chunk_sequence.h): Transforms an underlying sequence such that its elements are fixed-sized chunks.
   * [sliding window sequence](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/sliding_window_sequence.h): Transforms an underlying sequence such that its elements are a fixed-sized window slid over the underlying sequence.
   * [prefix pad sequence](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/prefix_pad_sequence.h): Pads an underlying sequence's prefix.
   * [suffix pad sequence](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/suffix_pad_sequence.h): Pads an underlying sequence's suffix.
   * [substring sequence](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/substring_sequence.h): Pulls a substring from an underlying sequence (truncates prefix and suffix).
   * [transform sequence](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/transform_sequence.h): Transforms the elements of a sequence via a user-supplied transformation function.
   * [zip sequence](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/zip_sequence.h): Brings a list of underlying sequences together, such that each element is a tuple of the corresponding elements in the underlying sequences.
   * [iota sequence](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/iota_sequence.h): Returns incrementally increasing integers as a sequence.
   * [mmap sequence](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/mmap_sequence.h): Returns the bytes of a memory mapped file as a sequence.
   
   These bundled sequences are building blocks for constructing new and more accurate pairwise sequence aligners. For example, if you have the [origin and terminus points](https://en.wikipedia.org/wiki/Circular_chromosome) of two bacterial genomes, you can [zip](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/zip_sequence.h) each with its corresponding genome sequence such that your pairwise alignment's scoring can account for [GC skew](https://en.wikipedia.org/wiki/GC_skew).

    </details>
    
 * <details><summary>Generic scoring.</summary>
 
   PAlignPrims's [scorer interface](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorer/scorer.h) is deeply flexible:

   * Elements scored using logic, not a substitution matrix.
   * Elements scored can be of different types (e.g., score an image against a number).
   * Resulting score can be any type vaguely resembling a number: Integers, floats, or [any type with the required type traits](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/concepts.h).
   
   PAlignPrims comes bundled with several scorer adapters and implementations, including ...
   
    * [PAM scorer](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorers/pam_scorer.h).
    * [BLOSUM scorer](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorers/blosum_scorer.h).
    * [QWERTY scorer](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorers/qwerty_scorer.h): Scoring based on where keys are positioned on a QWERTY keyboard.
    * [Levenshtein distance scorer](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorers/levenshtein_scorer.h): Scoring based on Levenshtein distance (string distance).
    * [single character substitution matrix scorer ](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorers/single_character_substitution_matrix_scorer.h): Loads an arbitrary substitution matrix encoded as a text table (single character elements).
    * [simple scorer](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorers/simple_scorer.h) / [constant scorer](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorers/constant_scorer.h) / [substitution scorer](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorers/substitution_map_scorer.h): Returns hardcoded scores based on criteria.
    * [consumption gating scorer](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorers/consumption_gating_scorer.h): Wraps an underlying scorer, gating progress based on how many sequence elements were consumed by the alignment.

   These bundled scorers are building blocks for constructing new and more accurate pairwise sequence aligners. For example, if you have the [origin and terminus points](https://en.wikipedia.org/wiki/Circular_chromosome) of two bacterial genomes, you can [zip](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/sequences/zip_sequence.h) each with its corresponding genome sequence and augment a DNA-based [single character substitution matrix scorer ](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/scorers/single_character_substitution_matrix_scorer.h) to account for [GC skew](https://en.wikipedia.org/wiki/GC_skew).

   </details>
   
 * <details><summary>Generic alignment graphs.</summary>

   PAlignPrims comes bundled with several [alignment graph interfaces](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/concepts.h), each intended to work with a specific variant of the [backtracking algorithm](https://github.com/offbynull/aligner/tree/main/offbynull/aligner/backtrackers): 

   * Graph: [graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graph/graph.h) / [backtracker](https://github.com/offbynull/aligner/tree/main/offbynull/aligner/backtrackers/graph_backtracker).
   * Pairwise alignment graph: [graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graph/pairwise_alignment_graph.h) / [backtracker](https://github.com/offbynull/aligner/tree/main/offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker).
   * Sliceable pairwise alignment graph: [graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graph/sliceable_pairwise_alignment_graph.h) / [backtracker](https://github.com/offbynull/aligner/tree/main/offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker).
   
   Each interface builds on the one before it, either constraining it or extending it based on the requirements of the accompanying backtracking algorithm. For more information, see documentation embedded within source files.
   
   PAlignPrims comes bundled with several graph adapters and implementations, including ...
 
    * [pairwise global alignment graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graphs/pairwise_global_alignment_graph.h).
    * [pairwise local alignment graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graphs/pairwise_local_alignment_graph.h).
    * [pairwise fitting alignment graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graphs/pairwise_fitting_alignment_graph.h).
    * [pairwise overlap alignment graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graphs/pairwise_overlap_alignment_graph.h).
    * [pairwise extended gap alignment graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph.h)
    *  [prefix sliceable pairwise alignment graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph.h): Prefix view of a sliceable pairwise alignment graph (suffix truncated).
    *  [suffix sliceable pairwise alignment graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph.h): Prefix view of a sliceable pairwise alignment graph (prefix truncated).
    *  [middle sliceable pairwise alignment graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph.h): Middle view of a sliceable pairwise alignment graph (prefix and suffix truncated).
    *  [reversed sliceable pairwise alignment graph](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph.h): Reversed view of a sliceable pairwise alignment graph (root becomes leaf and vice versa).
    
   These bundled graphs are building blocks for constructing new and more accurate pairwise sequence aligners. For example, the fitting alignment graph can easily be extended to perform [rotational alignment](https://github.com/offbynull/aligner/blob/main/offbynull/aligner/aligners/rotational_dynamic_programming_heap_aligner.h).

   </details>

 * <details><summary>Documented, tested, and many usage examples</summary>

   Virtually all header files ...

   * are documented with doxygen-style comments, detailing what each class is for, what each function does, and what each parameter is for.
   * come bundled with accompanying GUnit tests, intended both as a testing suite and as a usage example guide.
  
   </details>


 [Batteries included](https://en.wiktionary.org/wiki/batteries-included): PAlignPrims comes bundled with everything needed for pairwise aligners, including sequences, scorers, alignment graphs, and algorithms to operate on those alignment graphs.

