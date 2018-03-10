function(add_svg_png file size)
  add_custom_command(
      OUTPUT ${file}.png
      DEPENDS ${file}.svg
      COMMAND ${ImageMagick_convert_EXECUTABLE}
      ARGS "-size" "${size}" "-background" "transparent" "${file}.svg" "png32:${CMAKE_CURRENT_BINARY_DIR}/${file}.png"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      VERBATIM
    )
endfunction()

function(add_pen file)
  add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}.png
      DEPENDS ${file}.png
      COMMAND ${ImageMagick_convert_EXECUTABLE}
      ARGS "${file}.png" "-alpha" "copy" "-channel" "A" "-negate" "-channel"
      "RGB" "-fx" "#ffffff" "png32:${CMAKE_CURRENT_BINARY_DIR}/${file}.png"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      VERBATIM
    )
endfunction()

add_svg_png(camera 120x120)
add_svg_png(trash 120x120)
add_svg_png(palette 120x120)
add_pen(pen-small)
add_custom_target(data-png ALL DEPENDS camera.png palette.png trash.png pen-small.png)
