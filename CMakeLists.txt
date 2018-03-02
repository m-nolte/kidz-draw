cmake_minimum_required(VERSION 3.3)

project(draw)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)
find_package(ImageMagick REQUIRED COMPONENTS convert)
find_package(PNG REQUIRED)
find_package(Libmicrohttpd REQUIRED)

add_subdirectory(data)

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})
include_directories(${LIBMICROHTTPD_INCLUDE_DIR})

add_executable(draw draw.cc cursor.cc snapshots.cc)
target_link_libraries(draw ${SDL2_LIBRARIES})
target_link_libraries(draw ${PNG_LIBRARY})
target_link_libraries(draw ${LIBMICROHTTPD_LIBRARY})
target_link_libraries(draw stdc++fs)
