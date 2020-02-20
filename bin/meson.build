subdir('common')

bin_private_include_dir = include_directories('.')

boost_program_options_dep = dependency('boost', modules: ['program_options'])

executable(
  'beryl',
  sources: files(['beryl/beryl.cpp']),
  include_directories: [bin_private_include_dir],
  cpp_args: cpp_args,
  pie: true,
  link_args: link_args,
  dependencies: [jemalloc_dep, beryl_lib_dep, boost_program_options_dep],
  install: true)
