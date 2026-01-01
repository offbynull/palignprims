project(
        'palignforge',
        'cpp',
        version : '1.0.0',
        default_options : ['warning_level=everything', 'cpp_std=c++23']
)

# Configuration variables
# -----------------------
cc = meson.get_compiler('cpp')
dependencies = []
include_dirs = []
override_options = []
cpp_args = []
link_args = []

# Include Python dependency
# -------------------------
py = import('python')
py_installation = py.find_installation(
        pure: false,
        modules: []
)

# Include boost dependency
# ------------------------
# NOTE: Do not add dependency to dependencies list until building the target. Some targets may not need boost.
boost_modules = [
        'container',
        'filesystem',
        'iostreams'
]
boost_dep = dependency(
        'boost',
        version : '>=1.89.0',
        modules : boost_modules,
        static : true
)

# Load source checker target
# --------------------------
source_checker = custom_target(
        'source_checker',
        command: [py_installation.full_path(), meson.current_source_dir() + '/source_checker.py'],
        output: ['unused'],
        build_always_stale: true
)

# Load doxygen target
# -------------------
dot = find_program('dot', required: true)
doxygen = find_program('doxygen', required: true)
doc_generator = custom_target(
        'doc_generator',
        command: [py_installation.full_path(), meson.current_source_dir() + '/source_doxygen_runner.py'],
        output : ['docs'],
        build_always_stale: true
)

# Configure options
# -----------------
base_cpp_args = []
base_link_args = []
base_override_options = []
base_debug_override_options = base_override_options + ['b_lto=false', 'b_ndebug=false', 'debug=true', 'cpp_debugstl=true'] # + ['optimization=0', 'buildtype=debug']
base_release_override_options = base_override_options + ['b_lto=true', 'b_ndebug=true', 'debug=false', 'cpp_debugstl=false'] # + ['optimization=3', 'buildtype=release']
cc = meson.get_compiler('cpp')
if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
        # Configure base arguments for compiling / linking
        # ------------------------------------------------
        # Disable padding notification warning: Don't care about the compiler padding a struct.
        base_cpp_args += ['-Wno-padded']
        # Disable missing noexcept warning: Don't care that functions can be marked with noexcept.
        base_cpp_args += ['-Wno-noexcept']
        # Disable ignoring inline warning: When function implementations are placed inside of class definitions, it's a hint to the
        # compiler to inline the function. If it can't inline the function (or it doesn't inline it because it seems like a bad idea), it
        # puts out a -Winline warning. This only shows up when compiling at higher optimization levels.
        base_cpp_args += ['-Wno-inline']
        # Disable superfluous packing warning: When packing a struct, the compiler issues a warning if the struct is exactly the same
        # size in its packed form vs its padded form (because all members are already aligned). These warnings are being supressed because
        # the intention is for these structs to be packed, even if packing does nothing. In the future, should such a struct ever change
        # such that members are no longer aligned, the packing will ensure padding stays out.
        base_cpp_args += ['-Wno-packed']
        # Disable warnings specific to g++ extensions: These warnings are about applying extensions specific to g++. It doesn't apply to
        # portable C++ code. As such, don't care about them.
        base_cpp_args += ['-Wno-suggest-attribute=pure', '-Wno-suggest-attribute=const']
        # Unsure how to handle this, but it seems safe to ignore as this library requires a relatively new version of g++ and STL.
        base_cpp_args += ['-Wno-abi-tag']
        # Enable more robust compiler diagnostics
        base_cpp_args += ['-fconcepts-diagnostics-depth=9999', '-ftemplate-backtrace-limit=0']
        # Configure debug arguments for compiling / linking
        # ------------------------------------------------
        base_debug_cpp_args = base_cpp_args
        base_debug_link_args = base_link_args
        # Enable standard library debugging.
        base_debug_cpp_args += ['-D_GLIBCXX_DEBUG']
        # Enable Google sanititzers.
        # DON'T USE THIS? MESON HAS b_sanitize OPTION?
        # san_args = ['-fsanitize=undefined', '-fsanitize=thread']
        # if cc.get_id() == 'clang'
        #         # Only available in clang -- doesn't really matter because I use valgrind for these.
        #         san_args += ['-fsanitize=memory']
        #         san_args += ['-fsanitize=leak']
        #         san_args += ['-fsanitize=address']
        # endif
        # debug_cpp_args += san_args
        # debug_link_args += san_args
        # Configure release arguments for compiling / linking
        # ---------------------------------------------------
        base_release_cpp_args = base_cpp_args
        base_release_link_args = base_link_args
else
        base_debug_cpp_args = base_cpp_args + []
        base_debug_link_args = base_link_args + []
        base_release_cpp_args = base_cpp_args + []
        base_release_link_args = base_link_args + []
endif
# Set configuration options based on build type
# ---------------------------------------------
buildtype = get_option('buildtype')
if buildtype in ['debug', 'debugoptimized', 'minsize']  # Meson claims these profiles all have debugging info turned on, so enable debugging features
        override_options += base_debug_override_options
        cpp_args += base_debug_cpp_args
        link_args += base_debug_link_args
elif buildtype == 'release'
        override_options += base_release_override_options
        cpp_args += base_release_cpp_args
        link_args += base_release_link_args
elif buildtype == 'plain'  # Unsure about what to do for plain - keep as-is for now.
        override_options += []
        cpp_args += []
        link_args += []
else
        error('Unknown buildtype: ' + buildtype)
endif

# Create unittest executables
# ---------------------------
gtest = subproject('gtest')
gtest_dependency = gtest.get_variable('gtest_dep')
gmock_dependency = gtest.get_variable('gmock_dep')
gtest = executable(
        'gtest',
        [
                'gtest-all.cpp',
                'offbynull/helpers/unordered_thread_pool_test.cpp',
                'offbynull/helpers/simple_value_bidirectional_view_test.cpp',
                'offbynull/helpers/forkable_thread_pool_test.cpp',
                'offbynull/helpers/blankable_bidirectional_view_test.cpp',
                'offbynull/helpers/join_bidirectional_view_test.cpp',
                'offbynull/helpers/concat_bidirectional_view_test.cpp',
                'offbynull/aligner/graph/utils_test.cpp',
                'offbynull/aligner/graph/sliceable_pairwise_alignment_graph_test.cpp',
                'offbynull/aligner/graphs/directed_graph_test.cpp',
                'offbynull/aligner/graphs/grid_graph_test.cpp',
                'offbynull/aligner/graphs/pairwise_global_alignment_graph_test.cpp',
                'offbynull/aligner/graphs/pairwise_local_alignment_graph_test.cpp',
                'offbynull/aligner/graphs/pairwise_overlap_alignment_graph_test.cpp',
                'offbynull/aligner/graphs/pairwise_fitting_alignment_graph_test.cpp',
                'offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph_test.cpp',
                'offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph_test.cpp',
                'offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph_test.cpp',
                'offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph_test.cpp',
                'offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph_test.cpp',
                'offbynull/aligner/backtrackers/graph_backtracker/backtracker_test.cpp',
                'offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker_test.cpp',
                'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/path_container/path_container_test.cpp',
                'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker/forward_walker_test.cpp',
                'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker/bidi_walker_test.cpp',
                'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter/resident_segmenter_test.cpp',
                'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider/sliced_subdivider_test.cpp',
                'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker_test.cpp',
                'offbynull/aligner/sequences/mmap_sequence_test.cpp',
                'offbynull/aligner/sequences/iota_sequence_test.cpp',
                'offbynull/aligner/sequences/transform_sequence_test.cpp',
                'offbynull/aligner/sequences/reverse_sequence_test.cpp',
                'offbynull/aligner/sequences/prefix_pad_sequence_test.cpp',
                'offbynull/aligner/sequences/suffix_pad_sequence_test.cpp',
                'offbynull/aligner/sequences/substring_sequence_test.cpp',
                'offbynull/aligner/sequences/sliding_window_sequence_test.cpp',
                'offbynull/aligner/sequences/chunk_sequence_test.cpp',
                'offbynull/aligner/sequences/zip_sequence_test.cpp',
                'offbynull/aligner/scorers/constant_scorer_test.cpp',
                'offbynull/aligner/scorers/simple_scorer_test.cpp',
                'offbynull/aligner/scorers/levenshtein_scorer_test.cpp',
                'offbynull/aligner/scorers/substitution_map_scorer_test.cpp',
                'offbynull/aligner/scorers/single_character_substitution_matrix_scorer_test.cpp',
                'offbynull/aligner/scorers/blosum_scorer_test.cpp',
                'offbynull/aligner/scorers/pam_scorer_test.cpp',
                'offbynull/aligner/scorers/qwerty_scorer_test.cpp',
                'offbynull/aligner/scorers/widening_scorer_test.cpp',
                'offbynull/aligner/scorers/wrap_callable_scorer_test.cpp',
                'offbynull/aligner/aligners/global_dynamic_programming_heap_aligner_test.cpp',
                'offbynull/aligner/aligners/global_dynamic_programming_stack_aligner_test.cpp',
                'offbynull/aligner/aligners/global_sliced_subdivision_heap_aligner_test.cpp',
                'offbynull/aligner/aligners/global_sliced_subdivision_stack_aligner_test.cpp',
                'offbynull/aligner/aligners/local_dynamic_programming_heap_aligner_test.cpp',
                'offbynull/aligner/aligners/local_dynamic_programming_stack_aligner_test.cpp',
                'offbynull/aligner/aligners/local_sliced_subdivision_heap_aligner_test.cpp',
                'offbynull/aligner/aligners/local_sliced_subdivision_stack_aligner_test.cpp',
                'offbynull/aligner/aligners/overlap_dynamic_programming_heap_aligner_test.cpp',
                'offbynull/aligner/aligners/overlap_dynamic_programming_stack_aligner_test.cpp',
                'offbynull/aligner/aligners/overlap_sliced_subdivision_heap_aligner_test.cpp',
                'offbynull/aligner/aligners/overlap_sliced_subdivision_stack_aligner_test.cpp',
                'offbynull/aligner/aligners/fitting_dynamic_programming_heap_aligner_test.cpp',
                'offbynull/aligner/aligners/fitting_dynamic_programming_stack_aligner_test.cpp',
                'offbynull/aligner/aligners/fitting_sliced_subdivision_heap_aligner_test.cpp',
                'offbynull/aligner/aligners/fitting_sliced_subdivision_stack_aligner_test.cpp',
                'offbynull/aligner/aligners/extended_gap_dynamic_programming_heap_aligner_test.cpp',
                'offbynull/aligner/aligners/extended_gap_dynamic_programming_stack_aligner_test.cpp',
                'offbynull/aligner/aligners/extended_gap_sliced_subdivision_heap_aligner_test.cpp',
                'offbynull/aligner/aligners/extended_gap_sliced_subdivision_stack_aligner_test.cpp',
        ],
        source_checker,
        dependencies: dependencies + [gtest_dependency]+ [boost_dep],
        include_directories: include_dirs,
        override_options: override_options,
        cpp_args: cpp_args,
        link_args: link_args
)
test('gtest', gtest)

# Create benchmark executables
# ----------------------------
benchmark_tiny_alignment_000 = executable(
        'benchmark-tiny-alignment_000',
        ['benchmark-tiny-alignment.cpp'],
        source_checker,
        dependencies : [boost_dep],
        override_options : base_release_override_options,
        cpp_args : base_release_cpp_args \
                + ['-DBENCHMARK_HEAP', '-DBENCHMARK_SIZE_T_INDEX'],
        link_args : base_release_link_args
)
benchmark_tiny_alignment_0m00 = executable(
        'benchmark-tiny-alignment_0m00',
        ['benchmark-tiny-alignment.cpp'],
        source_checker,
        dependencies : [boost_dep],
        override_options : base_release_override_options,
        cpp_args : base_release_cpp_args \
                + ['-DBENCHMARK_HEAP', '-DMINIMIZE_ALLOCS', '-DBENCHMARK_SIZE_T_INDEX'],
        link_args : base_release_link_args
)
benchmark_tiny_alignment_100 = executable(
        'benchmark-tiny-alignment_100',
        ['benchmark-tiny-alignment.cpp'],
        source_checker,
        dependencies : [boost_dep],
        override_options : base_release_override_options,
        cpp_args : base_release_cpp_args \
                + ['-DBENCHMARK_STACK', '-DBENCHMARK_SIZE_T_INDEX'],
        link_args : base_release_link_args
)
benchmark_tiny_alignment_101 = executable(
        'benchmark-tiny-alignment_101',
        ['benchmark-tiny-alignment.cpp'],
        source_checker,
        dependencies : [boost_dep],
        override_options : base_release_override_options,
        cpp_args : base_release_cpp_args \
                + ['-DBENCHMARK_STACK', '-DBENCHMARK_SIZE_T_INDEX', '-DOBN_PACK_STRUCTS'],
        link_args : base_release_link_args
)
benchmark_tiny_alignment_110 = executable(
        'benchmark-tiny-alignment_110',
        ['benchmark-tiny-alignment.cpp'],
        source_checker,
        dependencies : [boost_dep],
        override_options : base_release_override_options,
        cpp_args : base_release_cpp_args \
                + ['-DBENCHMARK_STACK', '-DBENCHMARK_UINT8_INDEX'],
        link_args : base_release_link_args
)
benchmark_tiny_alignment_111 = executable(
        'benchmark-tiny-alignment_111',
        ['benchmark-tiny-alignment.cpp'],
        source_checker,
        dependencies : [boost_dep],
        override_options : base_release_override_options,
        cpp_args : base_release_cpp_args \
                + ['-DBENCHMARK_STACK', '-DBENCHMARK_UINT8_INDEX', '-DOBN_PACK_STRUCTS'],
        link_args : base_release_link_args
)