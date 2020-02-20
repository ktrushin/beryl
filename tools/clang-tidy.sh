#/bin/sh

set -e

clang_tidy_exe=$CLANG_TIDY
if [ -z "$clang_tidy_exe" ]; then
    clang_tidy_exe='clang-tidy'
fi

source_files=$(find bin/ include/ lib/ -name '*.[h|c]pp')
build_dir=_build_clang_tidy
rm -fr $build_dir
CXX='clang++-8' meson $build_dir
sed -i 's/-pipe//g' $build_dir/compile_commands.json
echo "starting clang-tidy"
clang-tidy-8 -p $build_dir --quiet $source_files 2>/dev/null
echo "clang-tidy has finished"
