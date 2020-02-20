gtest_proj = subproject('gtest')
gtest_main_dep = gtest_proj.get_variable('gtest_main_dep')

beryl_unit_sources = files([
  'beryl/resource_record_test.cpp',
  'beryl/read_zone_test.cpp',
  'beryl/domain_name_test.cpp',
  'beryl/domain_tree_test.cpp',
  'beryl/string_test.cpp',
  'beryl/tokenizer_test.cpp'
])

beryl_unit = executable(
  'beryl',
  sources: beryl_unit_sources,
  cpp_args: cpp_args,
  include_directories: include_directories(['../lib', '.']),
  link_args: link_args,
  dependencies : [
    jemalloc_dep,
    beryl_lib_dep,
    gtest_main_dep,
    unit_testing_dep],
  install : false)
test('beryl_unit', beryl_unit)
