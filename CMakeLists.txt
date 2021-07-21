cmake_minimum_required(VERSION 3.19.1)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

project(boson)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
add_definitions(${LLVM_DEFINITIONS})

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(boson src/bridge.cpp src/parser.cpp src/ast.cpp src/lexer.cpp src/codegen.cpp src/scope.cpp src/scopelist.cpp)

llvm_map_components_to_libnames(llvm_libs support  ${LLVM_TARGETS_TO_BUILD})
target_link_libraries(boson ${llvm_libs})
install(TARGETS boson DESTINATION bin)