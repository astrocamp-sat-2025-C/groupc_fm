# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/a81802/.pico-sdk/sdk/2.2.0/tools/pioasm"
  "/Users/a81802/Documents/Programs/test1/pioasm"
  "/Users/a81802/Documents/Programs/test1/pioasm-install"
  "/Users/a81802/Documents/Programs/test1/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/tmp"
  "/Users/a81802/Documents/Programs/test1/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp"
  "/Users/a81802/Documents/Programs/test1/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src"
  "/Users/a81802/Documents/Programs/test1/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/a81802/Documents/Programs/test1/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/a81802/Documents/Programs/test1/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
