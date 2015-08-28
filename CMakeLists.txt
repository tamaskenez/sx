cmake_minimum_required(VERSION 3.0)
project(sx)

include(CMakePrintHelpers)

if(NOT MSVC)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED 1)
endif()

file(GLOB_RECURSE files sx/*.h sx/*.cpp)

add_library(sx STATIC ${files})

target_include_directories(sx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<INSTALL_INTERFACE:include>
)

add_executable(playground playground.cpp)
target_link_libraries(playground sx)

install(TARGETS sx EXPORT sx-targets
    ARCHIVE DESTINATION lib)
install(EXPORT sx-targets FILE sx-config.cmake DESTINATION lib/cmake/sx)
install(DIRECTORY sx DESTINATION include)
