# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/ana/arduino-puppy/pwm_esc")
  file(MAKE_DIRECTORY "/home/ana/arduino-puppy/pwm_esc")
endif()
file(MAKE_DIRECTORY
  "/home/ana/arduino-puppy/pwm_esc/build/pwm_esc"
  "/home/ana/arduino-puppy/pwm_esc/build/_sysbuild/sysbuild/images/pwm_esc-prefix"
  "/home/ana/arduino-puppy/pwm_esc/build/_sysbuild/sysbuild/images/pwm_esc-prefix/tmp"
  "/home/ana/arduino-puppy/pwm_esc/build/_sysbuild/sysbuild/images/pwm_esc-prefix/src/pwm_esc-stamp"
  "/home/ana/arduino-puppy/pwm_esc/build/_sysbuild/sysbuild/images/pwm_esc-prefix/src"
  "/home/ana/arduino-puppy/pwm_esc/build/_sysbuild/sysbuild/images/pwm_esc-prefix/src/pwm_esc-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ana/arduino-puppy/pwm_esc/build/_sysbuild/sysbuild/images/pwm_esc-prefix/src/pwm_esc-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ana/arduino-puppy/pwm_esc/build/_sysbuild/sysbuild/images/pwm_esc-prefix/src/pwm_esc-stamp${cfgdir}") # cfgdir has leading slash
endif()
