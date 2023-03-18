# beryl

This is going to be a primitive DNS nameserver. The author started this pet
project in order gain practical knowledge of how DNS works. The project is at
the early stange of development: only a few classes and functions have been
implemented and augmented with unit tests.

### Developer Manual

Create and enter into the [Ganvigar](https://github.com/ktrushin/ganvigar)-based
development environment:
```
host_promtp> git clone git@github.com:ktrushin/ganvigar.git
host_promtp> git clone git@github.com:ktrushin/beryl.git
host_prompt> cd beryl
host_prompt> ../ganvigar/devenv-launch ganvigar/dev.conf
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
