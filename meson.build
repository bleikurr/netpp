project('netpp', 'cpp',
  default_options : ['cpp_std=c++20'],
  version : '0.0.1'
  )

subdir('src/')

includes = include_directories('include')

netpp_lib = library(
  'netpp',
  src_files,
  include_directories: includes,
  install: true
)

netpp_library_dep = declare_dependency(
  link_with: netpp_lib,
  include_directories: includes
)

