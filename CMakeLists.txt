cmake_minimum_required(VERSION 3.3)

project(draw)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)
find_package(ImageMagick REQUIRED COMPONENTS convert)
find_package(PNG REQUIRED)

add_subdirectory(data)

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})

add_executable(draw draw.cc cursor.cc)
target_link_libraries(draw ${SDL2_LIBRARIES})
target_link_libraries(draw ${PNG_LIBRARY})
