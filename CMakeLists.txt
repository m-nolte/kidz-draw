cmake_minimum_required(VERSION 3.3)

project(draw)

find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)

add_subdirectory(data)

include_directories(${SDL_INCLUDE_DIR})
include_directories(${SDLIMAGE_INCLUDE_DIR})

add_executable(draw draw.cc cursor.cc)
target_link_libraries(draw ${SDL_LIBRARY})
target_link_libraries(draw ${SDLIMAGE_LIBRARY})
