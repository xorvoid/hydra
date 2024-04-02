#!/usr/bin/env just --justfile

# List all available recipies
list:
  just --list

# Build dosbox (assumes MacOs)
build-dosbox:
  #!/bin/bash
  cd src/dosbox-x
  if [ -e src/dosbox-x ]; then
    make -j
  else
    ./build-macos-sdl2
  fi

# Build the local repository only
build-local:
  #!/bin/bash
  cd {{justfile_directory()}}
  if [ ! -d build ]; then
    meson setup build
  fi
  (cd build && ninja $@)

# Build all
build: build-dosbox build-local

# Test the repository
test: build
  #!/bin/bash
  cd {{justfile_directory()}}/build
  meson test
