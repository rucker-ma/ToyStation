include(FindPackageHandleStandardArgs)

if(NOT FFmpeg_FIND_COMPONENTS)
    set(FFmpeg_FIND_COMPONENTS AVCODEC AVFORMAT AVUTIL)
endif()

macro(set_component_found _component)
    if(${_component}_LIBRARY)
        set(${_component}_FOUND TRUE)
    else()
    # print error message
    endif()
endmacro()

macro(find_component _component)
    find_library(${_component}_LIBRARY NAMES ${_component} 
    HINTS ${CMAKE_SOURCE_DIR}/src/ThirdParty/ffmpeg-5.1.2/lib)
    set_component_found(${_component})
    mark_as_advanced(${_component}_LIBRARY)
endmacro()

if(NOT FFMPEG_LIBRARIES)
    find_component(avcodec)
    find_component(avdevice)
    find_component(avformat)
    find_component(avutil)
    find_component(postproc)
    find_component(swresample)
    foreach(_component ${FFmpeg_FIND_COMPONENTS})
        if(${_component}_FOUND)
            set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES} ${${_component}_LIBRARY})
        else()
        # print error message
        endif()        
    endforeach()
    
    set(FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES})
    set(FFMPEG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src/ThirdParty/ffmpeg-5.1.2/include)
    mark_as_advanced(FFMPEG_LIBRARIES FFMPEG_INCLUDE_DIR)
endif()

foreach (_component AVCODEC AVDEVICE AVFORMAT AVUTIL POSTPROCESS SWSCALE)
  set_component_found(${_component})
endforeach ()

set(_FFmpeg_REQUIRED_VARS FFMPEG_LIBRARIES)
foreach (_component ${FFmpeg_FIND_COMPONENTS})
  list(APPEND _FFmpeg_REQUIRED_VARS ${_component}_LIBRARY)
endforeach ()

find_package_handle_standard_args(FFmpeg DEFAULT_MSG ${_FFmpeg_REQUIRED_VARS})