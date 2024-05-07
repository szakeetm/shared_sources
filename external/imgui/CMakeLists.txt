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
  imgui_stdlib/imgui_stdlib.cpp
  ${IMGUI_TEST_ENGINE_SOURCES}
  )

target_include_directories( imgui
  INTERFACE ..
  PRIVATE . )

target_include_directories(imgui PRIVATE ../imgui_test_engine)

if(MSVC)
    find_package(OpenGL REQUIRED)
    # 1. link against external libs
    target_link_libraries(
            opengl32#.lib
            )
    target_include_directories(imgui PRIVATE ${SDL_DIR})
endif(MSVC)
if(NOT MSVC)
    if(APPLE)
        target_link_libraries(imgui PRIVATE "-framework AppKit")

    ENDIF()
endif(NOT MSVC)

find_package(OpenGL REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(imgui
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
target_link_libraries( imgui PUBLIC ${OPENGL_gl_LIBRARY})
