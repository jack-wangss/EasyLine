add_executable(editor main.cpp Log.cpp Renderer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/glad/src/glad.c)

target_include_directories(editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/glad/include
)

message("Source Dir: ${CMAKE_SOURCE_DIR}")

target_link_libraries(editor PRIVATE 
    imgui 
    glfw 
    OpenGL::GL 
    spdlog::spdlog
)

set_target_properties(editor PROPERTIES 
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "EasyLineEditor"
)

# Enable warnings as errors only for the editor target
if(MSVC)
    target_compile_definitions(editor PRIVATE _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
    target_compile_options(editor PRIVATE /WX)
else()
    target_compile_options(editor PRIVATE -Werror)
endif()

# Copy Resource folder next to the built executable so shaders and other assets are available at runtime
add_custom_command(TARGET editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../Resource"
        "$<TARGET_FILE_DIR:editor>/Resource"
    COMMENT "Copying Resource -> $<TARGET_FILE_DIR:editor>/Resource"
)