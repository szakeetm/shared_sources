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

# ImGui Test Engine
file(GLOB IMGUI_TEST_ENGINE_SOURCES 
		../imgui_test_engine/imgui*.cpp
	)

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
  ${IMGUI_TEST_ENGINE_SOURCES}
  )

target_include_directories( imgui
  INTERFACE ..
  PRIVATE . )

target_include_directories(imgui PRIVATE ../imgui_test_engine)

if(MSVC)
    # 1. link against external libs
    target_link_libraries(imgui PUBLIC
            opengl32#.lib
            )
endif(MSVC)
if(NOT MSVC)
    if(APPLE)
            target_link_libraries(imgui PRIVATE "-framework AppKit")
    endif()
endif(NOT MSVC)


find_package(OpenGL REQUIRED)
target_include_directories(imgui PUBLIC ${OPENGL_INCLUDE_DIRS})
find_package(SDL2 REQUIRED)
target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(imgui PUBLIC ${SDL2_LIBRARIES})
target_link_libraries(imgui PUBLIC ${SDL2_LIBRARY})


#target_compile_definitions( imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD )
target_link_libraries( imgui PUBLIC #[[glfw glad]] ${OPENGL_gl_LIBRARY})
