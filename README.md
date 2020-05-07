# beryl

This is going to be a primitive DNS nameserver. The author started
this pet project in order gain practical knowledge of how DNS works.
The project is at the very beginning: only a few classes and functions
have been implemented and augmented with unit tests.

The development is being done in a Docker container (based on Ubuntu 20.04)
where the necessary toolchain is installed. One can use either GCC or Clang
for compiling the project. The project uses [Meson](https://mesonbuild.com/)
as a build system. It provides much cleaner and more logical DSL than
CMake does. Unit tests are written with Google Test framework.

In regards to storing C++ code, placing everything into single `src` directory
is intentionally avoided. Instead, three separate directories are used:
`lib` and `bin` keep sources of the libraries and binaries respectively,
`include` is dedicated for public header files of the libraries.
Not olnly does it intoroduce a better layout and but also prevents too deep
directory nesting. Each of three directories mentioned above has a separate
subdirectory for each library/binary (only `ssi` at the moment). That makes
it easy to extend the project with other libraries/binaries just by placing
their subdirectories alongside with existing one.


### Developer Manual

Create a Docker image and run a container executing Bash. This requires Docker
18.09 or higher and downloads considerable amount of data from the Internet:
```
host_promtp> git clone git@github.com:ktrushin/beryl.git
host_prompt> cd beryl
host_prompt> ./tools/docker.sh
```
Run the formatter and the linter:  
```
container_prompt> ./tools/clang-format.sh
container_prompt> ./tools/clang-tidy.sh
```
Compile with GCC and run unit tests:
```
container_prompt> CXX='ccache g++' meson -Db_asneeded=false _build_gcc
container_prompt> ninja -v -C _build_gcc -j$(nproc)
container_prompt> ./_build_gcc/unit_tests/beryl
```
Compile with Clang and run unit tests:
```
container_prompt> CXX='ccache clang++' meson -Db_asneeded=false _build_clang
container_prompt> ninja -v -C _build_clang -j$(nproc)
container_prompt> ./_build_clang/unit_tests/beryl
```
