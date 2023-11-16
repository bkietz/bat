# list available recipes
default:
  @just --list --justfile {{justfile()}} --unsorted

# generate build/
cmake:
  cmake -S . \
    -G Ninja \
    -B build \
    -D CMAKE_BUILD_TYPE=Debug \
    -D CMAKE_EXPORT_COMPILE_COMMANDS=ON

# run tests
test +FILTER='*': build
  cd build/test; ctest

# build tests
build: cmake
  ninja -C build

link DIR:
  mkdir -p {{DIR}}
  rm -f {{DIR}}/bat {{DIR}}/src
  ln -s {{justfile_directory()}}/include {{DIR}}/bat
  ln -s {{justfile_directory()}}/src {{DIR}}/src

# wipe and re-generate build/
clean:
  rm -rf build
  @just cmake

ino SKETCH:
  arduino-cli compile \
    --upload --port /dev/ttyACM0 \
    --output-dir build/arduino/{{SKETCH}} \
    --fqbn adafruit:nrf52:feather52840sense \
    {{SKETCH}}/

compile_commands SKETCH:
  arduino-cli compile \
    --only-compilation-database \
    --build-path build/arduino/{{SKETCH}} \
    --fqbn adafruit:nrf52:feather52840sense \
    {{SKETCH}}/

  ln -s {{justfile_directory()}}/build/arduino/{{SKETCH}}/compile_commands.json \
    {{SKETCH}}/

monitor:
  arduino-cli monitor --port /dev/ttyACM0
