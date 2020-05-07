#!/bin/sh

############################### VERSIONING DATA ################################

# The project itself is versioned according to semantic versioning scheme
# (please see https://semver.org/)

major=0
minor=1
patch=0
prerelease=''
build_metadata=''

# The shared library should be versioned independently of
# project in order to establish backward compatibility on both
# semantical and _binary_ levels.
# Libtool's versioning system is used for that (please see
# https://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning).
# The shared library version comprises three components separated by a colon
# as follows: `current:revision:age`.
# However, the library file gets suffix `(current - age).revision.age`
# (please see https://github.com/pvanhoof/dir-examples),
# where value `(curent - age)` is a shared object version (aka soversion).

current=0
revision=0
age=0


############################### VERSIONING LOGIC ###############################
project_version="$major.$minor.$patch"
if [ ! -z "$prerelease" ]; then
  project_version="$project_version-$prerelease"
fi
if [ ! -z "$build_metadata" ]; then
  project_version="$project_version+$build_metadata"
fi

library_version="$current:$revision:$age"
soversion=$((current-age))
library_suffix="$soversion.$revision.$age"


############################# SCRIPT INPUT HANDLING ############################
prog="get_version.sh"

help_msg="\
Description:
  Print version of a specified entity.

Usage:
  $prog (--project|--library|--shared_object|--library_suffix)

Options:
  --project           Print project version.
  --library           Print shared library version in Libtool's format.
  --shared_object     Print shared object version (aka soversion).
  --library_suffix    Print library file suffix.
  -h, --help          Print this help message.

See also:
  Semantic Versioning:
    https://semver.org
  Libtool's versioning system:
    https://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning"

print_error() {
  echo "Error: $1\n$help_msg" >&2
}

if [ "$#" -ne 1 ]; then
  print_error "$prog must be provided with exactly one option."
  exit 1
fi

case "$1" in
  -h|--help) echo "$help_msg"; exit 0;;
  --project) echo "$project_version"; exit 0;;
  --library) echo "$library_version"; exit 0;;
  --shared_object) echo "$soversion"; exit 0;;
  --library_suffix) echo "$library_suffix"; exit 0;;
  *) print_error "unrecongnized input."; exit 1;;
esac

print_error "no option is specified"
exit 1
