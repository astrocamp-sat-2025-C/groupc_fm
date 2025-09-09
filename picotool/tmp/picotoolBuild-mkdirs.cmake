# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/a81802/Documents/Programs/test1/_deps/picotool-src"
  "/Users/a81802/Documents/Programs/test1/_deps/picotool-build"
  "/Users/a81802/Documents/Programs/test1/_deps"
  "/Users/a81802/Documents/Programs/test1/picotool/tmp"
  "/Users/a81802/Documents/Programs/test1/picotool/src/picotoolBuild-stamp"
  "/Users/a81802/Documents/Programs/test1/picotool/src"
  "/Users/a81802/Documents/Programs/test1/picotool/src/picotoolBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/a81802/Documents/Programs/test1/picotool/src/picotoolBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/a81802/Documents/Programs/test1/picotool/src/picotoolBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
