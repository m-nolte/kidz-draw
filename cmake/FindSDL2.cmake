find_path(SDL2_INCLUDE_DIR SDL.h HINTS ENV SDL2DIR PATH_SUFFIXES SDL2 include/SDL2 include)
find_library(SDL2_LIBRARY NAMES SDL2)

if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
  set(SDL2_LIBRARIES ${SDL2_LIBRARY})
  foreach(component ${SDL2_FIND_COMPONENTS})
    find_library(SDL2_${component}_LIBRARY NAMES SDL2_${component})
    if(SDL2_${component}_LIBRARY AND EXISTS "${SDL2_INCLUDE_DIR}/SDL_${component}.h")
      set(SDL2_${component}_FOUND TRUE)
    else()
      set(SDL2_${component}_FOUND FALSE)
    endif()

    mark_as_advanced(SDL2_${component}_LIBRARY)
    set(SDL2_LIBRARIES ${SDL2_LIBRARIES} "${SDL2_${component}_LIBRARY}")
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2 REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR HANDLE_COMPONENTS)

mark_as_advanced(SDL2_IMAGE_LIBRARIES SDL_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR)
