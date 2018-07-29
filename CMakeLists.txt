cmake_minimum_required(VERSION 2.8)
project(json-messagepack-converter)

set(CMAKE_CXX_STANDARD 11)

include_directories("${PROJECT_SOURCE_DIR}/thirdparty/picojson")
include_directories("${PROJECT_SOURCE_DIR}/thirdparty/msgpack-c/include")
link_directories("${PROJECT_SOURCE_DIR}/build/lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s RESERVED_FUNCTION_POINTERS=20")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXTRA_EXPORTED_RUNTIME_METHODS=[]")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --pre-js ${PROJECT_SOURCE_DIR}/src/jmc1.js")

add_executable(jmc src/jmc.cpp)
target_link_libraries(jmc msgpackc)

install(TARGETS jmc
  RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/www
  LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/www
  ARCHIVE DESTINATION ${PROJECT_SOURCE_DIR}/www)
  
