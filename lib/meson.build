lib_private_include_dir = include_directories('.')

beryl_lib_sources = files([
  'beryl/read_zone.cpp',
  'beryl/domain_name.cpp'
])

boost_date_time_dep =  dependency('boost', modules: ['date_time'])
boost_thread_dep = dependency('boost', modules: ['thread', 'system'])

beryl_lib = library(
  'beryl',
  version: run_command(get_version, '--library_suffix').stdout().strip(),
  sources: beryl_lib_sources,
  include_directories: [public_include_dir, lib_private_include_dir],
  cpp_args: cpp_args,
  link_args: link_args,
  dependencies: [jemalloc_dep, boost_date_time_dep, boost_thread_dep],
  install: true)

pkg_mod = import('pkgconfig')
pkg_mod.generate(beryl_lib, description: 'A primitive DNS nameserver - library')

beryl_lib_dep = declare_dependency(
  include_directories: public_include_dir,
  link_with: beryl_lib,
  dependencies: [boost_date_time_dep])
