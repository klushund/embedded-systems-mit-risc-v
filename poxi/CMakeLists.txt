# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS 
  "C:/Users/prits/git/embedded-systems-mit-risc-v/components/components/graphics" 
  "C:/Users/prits/git/embedded-systems-mit-risc-v/components/components/pushbtn"
  "C:/Users/prits/git/embedded-systems-mit-risc-v/components/components/max3010x"
  "C:/Users/prits/git/embedded-systems-mit-risc-v/components/components/filter"
  "C:/Users/prits/git/embedded-systems-mit-risc-v/components/components/ringbuffer")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(app-template)
