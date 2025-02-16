project('untitled', 'cpp',
        version : '0.0.1',
        default_options : ['warning_level=everything', 'cpp_std=c++23'])

boost_dep = dependency('boost', modules : ['container', 'filesystem', 'iostreams'])

example = executable(
        'untitled',
        ['main.cpp'],
        dependencies : [boost_dep],
        cpp_args : ['-fconcepts-diagnostics-depth=9999'], #, '-D_GLIBCXX_DEBUG'],
        # override_options : ['b_lto=true'],
        install : true
)
test('test', example)


#library = shared_library(
#        'obnaligner',
#        sources: ['offbynull/aligner/library.cpp'],
#        dependencies: [boost_dep],
#        install: true,
#        version: meson.project_version(),
#        soversion: meson.project_version()
#)



python3 = find_program('python3', required : false)
if not python3.found()
        warning('Python3 not found -- some non-critical tasks may be skipped')
endif
dot = find_program('dot', required : false)
if not dot.found()
        warning('Graphviz (dot) not found -- some non-critical tasks may be skipped')
endif
doxygen = find_program('doxygen', required : false)
if not doxygen.found()
        warning('doxygen not found -- some non-critical tasks may be skipped')
endif

if python3.found()
        source_checker = custom_target(
                'source_checker',
                command: ['python3', meson.current_source_dir() + '/source_checker.py'],
                output: ['unused'],
                build_always_stale: true
        )
else
        warning('Disabling source-check due to missing tools')
        source_check = []
endif

if python3.found() and dot.found() and doxygen.found()
        doc_generator = custom_target(
                'doc_generator',
                command: ['python3', meson.current_source_dir() + '/source_doxygen_runner.py'],
                output : ['docs'],
                build_always_stale: true
        )
else
        warning('Disabling doc-generate due to missing tools')
        doc_generator = []
endif

gtest = subproject('gtest')
gtest_dep = gtest.get_variable('gtest_dep')
gmock_dep = gtest.get_variable('gmock_dep')
tests_src = [
        'gtest-all.cpp',
        'offbynull/helpers/unordered_thread_pool_test.cpp',
#        'offbynull/helpers/forkable_thread_pool_test.cpp',
#        'offbynull/helpers/blankable_bidirectional_view_test.cpp',
#        'offbynull/helpers/join_bidirectional_view_test.cpp',
#        'offbynull/helpers/concat_view_test.cpp',
#        'offbynull/aligner/graph/utils_test.cpp',
         'offbynull/aligner/graph/sliceable_pairwise_alignment_graph_test.cpp',
         'offbynull/aligner/graph/multithreaded_sliceable_pairwise_alignment_graph_test.cpp',
#        'offbynull/aligner/graphs/directed_graph_test.cpp',
#        'offbynull/aligner/graphs/grid_graph_test.cpp',
#        'offbynull/aligner/graphs/pairwise_global_alignment_graph_test.cpp',
#        'offbynull/aligner/graphs/pairwise_local_alignment_graph_test.cpp',
#        'offbynull/aligner/graphs/pairwise_overlap_alignment_graph_test.cpp',
#        'offbynull/aligner/graphs/pairwise_fitting_alignment_graph_test.cpp',
#        'offbynull/aligner/graphs/pairwise_extended_gap_alignment_graph_test.cpp',
#        'offbynull/aligner/graphs/prefix_sliceable_pairwise_alignment_graph_test.cpp',
#        'offbynull/aligner/graphs/reversed_sliceable_pairwise_alignment_graph_test.cpp',
#        'offbynull/aligner/graphs/suffix_sliceable_pairwise_alignment_graph_test.cpp',
#        'offbynull/aligner/graphs/middle_sliceable_pairwise_alignment_graph_test.cpp',
#        'offbynull/aligner/backtrackers/graph_backtracker/backtracker_test.cpp',
#        'offbynull/aligner/backtrackers/pairwise_alignment_graph_backtracker/backtracker_test.cpp',
#        'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/path_container_test.cpp',
#        'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/forward_walker_test.cpp',
#        'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker_test.cpp',
#        'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/resident_segmenter_test.cpp',
#        'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider_test.cpp',
#        'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker_test.cpp',
##        'offbynull/aligner/backtrackers/multithreaded_sliceable_pairwise_alignment_graph_backtracker/bidi_walker_test.cpp',
##        'offbynull/aligner/backtrackers/multithreaded_sliceable_pairwise_alignment_graph_backtracker/diagonal_slice_slot_container_test.cpp',
##        'offbynull/aligner/backtrackers/multithreaded_sliceable_pairwise_alignment_graph_backtracker/diagonal_slice_slot_container_triplet_test.cpp',
#        'offbynull/aligner/sequences/mmap_sequence_test.cpp',
#        'offbynull/aligner/sequences/iota_sequence_test.cpp',
#        'offbynull/aligner/sequences/transform_sequence_test.cpp',
#        'offbynull/aligner/sequences/reversed_sequence_test.cpp',
#        'offbynull/aligner/sequences/prefix_pad_sequence_test.cpp',
#        'offbynull/aligner/sequences/suffix_pad_sequence_test.cpp',
#        'offbynull/aligner/sequences/substring_sequence_test.cpp',
#        'offbynull/aligner/sequences/sliding_window_sequence_test.cpp',
#        'offbynull/aligner/sequences/chunked_sequence_test.cpp',
#        'offbynull/aligner/sequences/zip_sequence_test.cpp',
#        'offbynull/aligner/scorers/constant_scorer_test.cpp',
#        'offbynull/aligner/scorers/simple_scorer_test.cpp',
#        'offbynull/aligner/scorers/levenshtein_scorer_test.cpp',
#        'offbynull/aligner/scorers/substitution_map_scorer_test.cpp',
#        'offbynull/aligner/scorers/single_character_substitution_matrix_scorer_test.cpp',
#        'offbynull/aligner/scorers/blosum45_scorer_test.cpp',
#        'offbynull/aligner/scorers/blosum50_scorer_test.cpp',
#        'offbynull/aligner/scorers/blosum62_scorer_test.cpp',
#        'offbynull/aligner/scorers/blosum80_scorer_test.cpp',
#        'offbynull/aligner/scorers/blosum90_scorer_test.cpp',
#        'offbynull/aligner/scorers/pam30_scorer_test.cpp',
#        'offbynull/aligner/scorers/pam70_scorer_test.cpp',
#        'offbynull/aligner/scorers/pam250_scorer_test.cpp',
#        'offbynull/aligner/scorers/qwerty_scorer_test.cpp',
##        'offbynull/aligner/aligner_test.cpp'
]
base_cpp_args = []
base_link_args = []
base_override_options = []
base_debug_cpp_args = []
base_debug_link_args = []
base_debug_override_options = base_override_options + ['optimization=0', 'buildtype=debug', 'b_lto=false', 'b_ndebug=false', 'debug=true', 'cpp_debugstl=true']
base_release_cpp_args = []
base_release_link_args = []
base_release_override_options = base_override_options +['optimization=3', 'buildtype=release', 'b_lto=true', 'b_ndebug=true', 'debug=false', 'cpp_debugstl=false']
cc = meson.get_compiler('cpp')
if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
        # No -W-padded
        # ------------
        # Don't care about the compiler padding a struct.
        base_cpp_args += ['-Wno-padded']
        # No -W-noexcept
        # -------------
        # Don't care that functions can be marked with noexcept.
        base_cpp_args += ['-Wno-noexcept']
        # No -W-inline
        # ------------
        # When function implementations are placed inside of class definitions, it's a hint to the compiler to inline the function. If it
        # can't inline the function (or it doesn't inline it because it seems like a bad idea), it puts out a -Winline warning. This only
        # shows up when compiling at higher optimization levels.
        base_cpp_args += ['-Wno-inline']
        # No -W-packed
        # -------------
        # When packing a struct, the compiler issues a warning if the struct is exactly the same size in its packed form vs its padded
        # form (because all members are already aligned). These warnings are being supressed because the intention is for these structs to
        # be packed, even if packing does nothing. In the future, should such a struct ever change such that members are no longer aligned,
        # the packing will ensure padding stays out.
        base_cpp_args += ['-Wno-packed']
        # No warnings for non-portable features
        # -------------------------------------
        # These warnings are about applying extensions specific to g++. It doesn't apply to portable C++ code. As such, don't care about
        # them.
        base_cpp_args += ['-Wno-suggest-attribute=pure', '-Wno-suggest-attribute=const']
        # No abi warnings
        # ---------------
        # Unsure how to handle this, but it seems safe to ignore as this library requires a relatively new version of g++ and STL.
        base_cpp_args += ['-Wno-abi-tag']
        # Compiler diagnostics
        # --------------------
        base_cpp_args += ['-fconcepts-diagnostics-depth=9999', '-ftemplate-backtrace-limit=0']

        base_debug_cpp_args = base_cpp_args + base_debug_cpp_args  # Move in base args that were decided in the blocks above
        base_debug_link_args = base_cpp_args + base_debug_cpp_args  # Move in base args that were decided in the blocks above
        # Google sanititzers
        # ------------------
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
        # Extra standard library debugging
        # --------------------------------
        base_debug_cpp_args += ['-D_GLIBCXX_DEBUG']

        base_release_cpp_args = base_cpp_args + base_release_cpp_args  # Move in base args that were decided in the blocks above
        base_release_link_args = base_cpp_args + base_release_cpp_args  # Move in base args that were decided in the blocks above
endif
test_exec = executable(
        'gtest-all_debug',
        tests_src,
        source_checker,
        # doc_generator,
        override_options : base_debug_override_options,
        dependencies : [boost_dep, gtest_dep, gmock_dep],
        cpp_args : base_debug_cpp_args \
                + ['-DOBN_PACK_STRUCTS'],  # Pack structs -- done to ensure code doesn't crash when structs are packed
        link_args : base_debug_link_args
)
test_exec_release = executable(
        'gtest-all_release',
        tests_src,
        source_checker,
        # doc_generator,
        dependencies : [boost_dep, gtest_dep, gmock_dep],
        override_options : base_release_override_options,
        cpp_args : base_release_cpp_args \
                + ['-DOBN_PACK_STRUCTS'],  # Pack structs -- done to ensure code doesn't crash when structs are packed
        link_args : base_release_link_args
)
# test('gtest tests', test_exec)


# Benchmarks
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