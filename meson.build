project('sudoku_solver', 'cpp',
    default_options : ['cpp_std=c++20'])

if get_option('buildtype') == 'release'
    # disable assertions from <cassert>
    add_project_arguments('-DNDEBUG', language: 'cpp')
endif

executable('sudoku_solver', 'main.cpp')
# executable('sudoku_solver_old', 'scratch/sudoku_solver.cpp')
