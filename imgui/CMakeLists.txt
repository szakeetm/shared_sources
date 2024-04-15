#
# Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

add_library( imgui STATIC
  imconfig.h
  imgui.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui.h
  imgui_impl_opengl2.cpp
  imgui_impl_opengl2.h
  imgui_impl_sdl2.cpp
  imgui_impl_sdl2.h
  imgui_internal.h
  imgui_tables.cpp
  imgui_widgets.cpp
  imstb_rectpack.h
  imstb_textedit.h
  imstb_truetype.h
  )

target_include_directories( imgui
  INTERFACE ..
  PRIVATE . )

if(MSVC)
    find_package(OpenGL REQUIRED)
    # 1. link against external libs
    target_link_libraries(imgui PUBLIC
            SDL2.lib
            SDL2main.lib
            opengl32#.lib
            )
    set(SDL_DIR ../../include/windows_only/SDL2)
    target_include_directories(imgui PRIVATE ${SDL_DIR})
endif(MSVC)
if(NOT MSVC)
    if(APPLE)
        #link to self-build sdl shared lib
        target_link_libraries(imgui PRIVATE "-framework AppKit")
        #target_link_libraries(${PROJECT_NAME} "-framework SDL2")

        FIND_PACKAGE(SDL2 REQUIRED)
        Message("")
        Message( STATUS "FINDING SDL2" )
        IF (${SDL2_FOUND})
            Message( STATUS "SDL2_FOUND: " ${SDL2_FOUND})
            Message( STATUS "SDL2_INCLUDE_DIR:" ${SDL2_INCLUDE_DIR})
            Message( STATUS "SDL2_LIBRARY: " ${SDL2_LIBRARY})
        ELSE()
            Message( STATUS "SDL2_FOUND: " ${SDL2_FOUND})
            Message( FATAL_ERROR "SDL2 NOT FOUND" )
        ENDIF()
    endif()

    find_package(OpenGL REQUIRED)
    target_include_directories(imgui PUBLIC ${OPENGL_INCLUDE_DIRS})

    find_package(SDL2 REQUIRED)
    target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})
    target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIR})

    target_link_libraries(imgui PUBLIC ${OPENGL_LIBRARIES})
    target_link_libraries(imgui PUBLIC ${SDL2_LIBRARIES})
    target_link_libraries(imgui PUBLIC ${SDL2_LIBRARY})
endif(NOT MSVC)

message(¨Inc ${SDL2_INCLUDE_DIR}¨)
message(¨Incs ${SDL2_INCLUDE_DIRS}¨)
message(¨Libs ${SDL2_LIBRARIES}¨)
message(¨Lib ${SDL2_LIBRARY}¨)

#target_compile_definitions( imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD )
target_link_libraries( imgui PUBLIC #[[glfw glad]] ${OPENGL_gl_LIBRARY})
