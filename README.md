# beryl

This is going to be a primitive DNS nameserver. The author started
this pet project in order gain practical knowledge of how DNS works.
The project is at the very beginning: only a few classes and functions
have been implemented and augmented with unit tests.

The development is being done in a Docker container (based on Ubuntu 18.04)
where the necessary toolchain is installed. One can use either GCC or Clang
for compiling the project. The project uses [Meson](https://mesonbuild.com/)
as a build system. Unit tests are written with Google Test framework.

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
container_prompt> CLANG_FORMAT='clang-format-8' ./tools/clang-format.sh
container_prompt> CLANG_TIDY='clang-tidy-8' ./tools/clang-tidy.sh
```
Compile with GCC and run unit tests:
```
container_prompt> CXX='ccache g++-8' meson -Db_asneeded=false _build_gcc
container_prompt> ninja -v -C _build_gcc -j$(nproc)
container_prompt> ./_build_gcc/unit_tests/beryl
```
Compile with Clang and run unit tests:
```
container_prompt> CXX='ccache clang++-8' meson -Db_asneeded=false _build_clang
container_prompt> ninja -v -C _build_clang -j$(nproc)
container_prompt> ./_build_clang/unit_tests/beryl
```
