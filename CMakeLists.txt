cmake_minimum_required(VERSION 3.3)

project(draw)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)
find_package(ImageMagick REQUIRED COMPONENTS convert)
find_package(PNG REQUIRED)
find_package(Libmicrohttpd REQUIRED)

add_subdirectory(tools)

include_directories(${SDL2_INCLUDE_DIR})
include_directories(${PNG_INCLUDE_DIR})
include_directories(${LIBMICROHTTPD_INCLUDE_DIR})

add_subdirectory(data)

function(add_embedded input symbol output)
  add_custom_command(
      OUTPUT ${output}
      DEPENDS ${input} embed data-png
      COMMAND $<TARGET_FILE:embed>
      ARGS ${input} ${symbol} ${output}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      VERBATIM
    )
endfunction()

add_embedded(${CMAKE_CURRENT_BINARY_DIR}/data/camera.png camera camera.cc)
add_embedded(${CMAKE_CURRENT_BINARY_DIR}/data/trash.png trash trash.cc)
add_embedded(${CMAKE_CURRENT_BINARY_DIR}/data/pen-small.png pen_small pen-small.cc)

add_executable(kidz-draw draw.cc cursor.cc snapshots.cc mycursor.cc camera.cc trash.cc pen-small.cc)
target_link_libraries(kidz-draw ${SDL2_LIBRARIES})
target_link_libraries(kidz-draw ${PNG_LIBRARY})
target_link_libraries(kidz-draw ${LIBMICROHTTPD_LIBRARY})
target_link_libraries(kidz-draw stdc++fs)

install(TARGETS kidz-draw DESTINATION bin)
