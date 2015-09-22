cmake_minimum_required(VERSION 2.8.7)
project(sx)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    include(CMakePrintHelpers)
endif()

if(NOT MSVC)
    if(NOT CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 14)
    endif()
    if(CMAKE_VERSION VERSION_LESS 3.1)
        add_definitions(-std=c++${CMAKE_CXX_STANDARD})
    else()
        set(CMAKE_CXX_STANDARD_REQUIRED 1)
    endif()
endif()

file(GLOB_RECURSE files sx/*.h sx/*.cpp)

add_library(sx STATIC ${files})

if(CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories(${CMAKE_CURRENT_LIST_DIR})
else()
    target_include_directories(sx PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
        $<INSTALL_INTERFACE:include>)
endif()

add_executable(playground playground.cpp)
target_link_libraries(playground sx)

install(TARGETS sx EXPORT sx-targets
    ARCHIVE DESTINATION lib)
install(EXPORT sx-targets FILE sx-config.cmake DESTINATION lib/cmake/sx)
install(DIRECTORY sx DESTINATION include)

if(SX_ENABLE_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif()
