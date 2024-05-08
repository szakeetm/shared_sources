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

add_library( implot STATIC
  implot.cpp
  implot_demo.cpp
  implot.h
  implot_internal.h
  implot_items.cpp
  )

set(IMGUI_DIR ../imgui)

target_include_directories( implot PRIVATE
        ${IMGUI_DIR}
        )

target_link_libraries(implot PUBLIC
        imgui
        )

#[[
message(¨Inc ${SDL2_INCLUDE_DIR}¨)
message(¨Incs ${SDL2_INCLUDE_DIRS}¨)
message(¨Libs ${SDL2_LIBRARIES}¨)
message(¨Lib ${SDL2_LIBRARY}¨)
]]

#target_compile_definitions( implot PRIVATE implot_IMPL_OPENGL_LOADER_GLAD )
target_link_libraries( implot PUBLIC #[[glfw glad]] #[[${OPENGL_gl_LIBRARY}]])
