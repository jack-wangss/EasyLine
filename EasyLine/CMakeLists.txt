cmake_minimum_required(VERSION 3.15)

project(EasyLine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Prefer out-of-source builds
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
	message(WARNING "It is recommended to build out-of-source.")
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add third-party libraries that provide CMake
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glfw ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/glfw_build)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/spdlog_build)

# ImGui (build from sources in 3rdparty/imgui)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui)
set(IMGUI_SOURCES
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
	${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
	${IMGUI_DIR}
	${IMGUI_DIR}/backends
)
# Use ImGui's built-in OpenGL loader (imgui_impl_opengl3_loader.h) by default
# Link to glfw and OpenGL so consumers don't have to
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# Add editor subdirectory
add_subdirectory(editor)
