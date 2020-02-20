#/bin/sh

set -e

clang_format_exe=$CLANG_FORMAT
if [ -z "$clang_format_exe" ]; then
    clang_format_exe='clang-format'
fi

source_files=$(find bin/ include/ lib/ unit_tests/ -name '*.[h|c]pp')
echo "starting clang-format"
$clang_format_exe -style=file -i $source_files
echo "clang-format has finished, run \`git diff\` to see the changes"
