project('untitled', 'cpp',
        version : '0.0.1',
        default_options : ['warning_level=everything', 'cpp_std=c++23'])

cc = meson.get_compiler('cpp')
if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
        add_project_arguments('-Wno-padded', language : 'cpp')
        add_project_arguments('-Wno-noexcept', language : 'cpp')
endif

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


gtest = subproject('gtest')
gtest_dep = gtest.get_variable('gtest_dep')
gmock_dep = gtest.get_variable('gmock_dep')
tests_src = [
        'gtest-all.cpp',
#        'offbynull/helpers/forward_range_join_view_test.cpp',
#        'offbynull/helpers/concat_view_test.cpp',
#        'offbynull/helpers/cartesian_product_2d_view_test.cpp',
#        'offbynull/aligner/graph/utils_test.cpp',
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
        'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_walker_test.cpp',
        'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/bidi_walker_test.cpp',
        'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/sliced_subdivider_test.cpp',
#        'offbynull/aligner/backtrackers/sliceable_pairwise_alignment_graph_backtracker/backtracker_test.cpp',
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
##        'offbynull/aligner/aligner_test.cpp'
]
test_exec = executable(
        'gtest-all',
        tests_src,
        dependencies : [boost_dep, gtest_dep, gmock_dep],
        cpp_args : ['-fconcepts-diagnostics-depth=9999', '-ftemplate-backtrace-limit=0', '-D_GLIBCXX_DEBUG']
)
test('gtest tests', test_exec)