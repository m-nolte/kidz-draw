function(add_inkscape_png)
  foreach(arg ${ARGN})
    add_custom_command(
      OUTPUT ${arg}.png
      DEPENDS ${arg}.svg
      COMMAND inkscape
      ARGS --export-width=120 --export-height=120 --export-png=${CMAKE_CURRENT_BINARY_DIR}/${arg}.png ${arg}.svg
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      VERBATIM
    )
  endforeach()
endfunction()

add_inkscape_png(camera trash)
add_custom_target(data-png ALL DEPENDS camera.png trash.png)

configure_file(arrow.cur draw.cur COPYONLY)

foreach(IMAGE pen-small.png)
  configure_file(${IMAGE} ${IMAGE} COPYONLY)
endforeach()
