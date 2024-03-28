#!/usr/bin/env just --justfile

# List all available recipies
list:
  just --list

# Build dosbox (assumes MacOs)
build-dosbox:
  #!/bin/bash
  cd dosbox-x
  make -j

# Build the repository
build:
  #!/bin/bash
  cd {{justfile_directory()}}
  if [ ! -d build ]; then
    meson setup build
  fi
  (cd build && ninja $@)

# Test the repository
test: build
  #!/bin/bash
  cd {{justfile_directory()}}/build
  meson test
