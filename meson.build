project('untitled', 'cpp',
        version : '0.0.1',
        default_options : ['warning_level=everything', 'cpp_std=c++23'])


boost_dep = dependency('boost', modules : ['container'])

example = executable(
        'untitled',
        ['main.cpp'],
        dependencies : [boost_dep],
        cpp_args : '-fconcepts-diagnostics-depth=9999',
        install : true
)
test('test', example)


library = shared_library(
        'obnaligner',
        sources: ['library.cpp'],
        dependencies: [boost_dep],
        install: true,
        version: meson.project_version(),
        soversion: meson.project_version()
)


gtest = subproject('gtest')
gtest_dep = gtest.get_variable('gtest_dep')
gmock_dep = gtest.get_variable('gmock_dep')
tests_src = [
        'tests/gtest-all.cpp',
        'tests/test_directed_graph.cpp',
        'tests/test_grid_graph.cpp',
        'tests/test_pairwise_local_alignment_graph.cpp',
        'tests/test_pairwise_global_alignment_graph.cpp',
        'tests/test_pairwise_fitting_alignment_graph.cpp',
        'tests/test_pairwise_extended_gap_alignment_graph.cpp',
        'tests/test_backtracker.cpp',
        'tests/test_backtracker_2.cpp',
]
test_exec = executable(
        'gtest-all',
        tests_src,
        dependencies : [boost_dep, gtest_dep, gmock_dep],
        cpp_args : ['-D_GLIBCXX_DEBUG']
)
test('gtest tests', test_exec)