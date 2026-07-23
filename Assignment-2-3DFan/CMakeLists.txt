cmake_minimum_required(VERSION 3.20)

project(Fan3D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(ImportDependencies)
import_dependencies()

set(FAN3D_SCENE_SOURCES
    Scene/Fan.cpp
    Scene/Renderer.cpp
    Scene/Transform.cpp
)

add_executable(Fan3D
    Scene/main.cpp
    ${FAN3D_SCENE_SOURCES}
)

target_include_directories(Fan3D PRIVATE Scene)
target_link_libraries(Fan3D PRIVATE glm::glm)

if(EMSCRIPTEN)
    target_compile_definitions(Fan3D PRIVATE USE_GLFW)
    target_link_options(Fan3D PRIVATE
        "-sUSE_GLFW=3"
        "-sUSE_WEBGL2=1"
        "-sFULL_ES3=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sASSERTIONS=1"
        "--preload-file=${CMAKE_CURRENT_LIST_DIR}/android/app/src/main/assets/shader@assets/shader"
        "--shell-file=${CMAKE_CURRENT_LIST_DIR}/web_shell.html"
    )
    set_target_properties(Fan3D PROPERTIES
        OUTPUT_NAME index
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/web"
    )
else()
    target_compile_definitions(Fan3D PRIVATE PLATFORM_WINDOWS GLEW_STATIC)
    target_link_libraries(Fan3D PRIVATE glfw libglew_static)

    if(MSVC)
        target_compile_options(Fan3D PRIVATE /W4 /permissive-)
        set_source_files_properties(Scene/Transform.cpp PROPERTIES
            COMPILE_OPTIONS "/wd4100;/wd4244"
        )
    else()
        target_compile_options(Fan3D PRIVATE -Wall -Wextra -Wpedantic)
        set_source_files_properties(Scene/Transform.cpp PROPERTIES
            COMPILE_OPTIONS "-Wno-unused-parameter"
        )
    endif()

    add_custom_command(TARGET Fan3D POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_LIST_DIR}/android/app/src/main/assets"
            "$<TARGET_FILE_DIR:Fan3D>/assets"
        COMMENT "Copying shader assets"
    )
    set_target_properties(Fan3D PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Fan3D>"
    )

    include(CTest)
    if(BUILD_TESTING)
        add_executable(FanInteractionTests
            tests/FanInteractionTests.cpp
            Scene/Fan.cpp
            Scene/Transform.cpp
        )
        target_include_directories(FanInteractionTests PRIVATE Scene)
        target_compile_definitions(FanInteractionTests PRIVATE PLATFORM_WINDOWS GLEW_STATIC)
        target_link_libraries(FanInteractionTests PRIVATE glm::glm glfw libglew_static)
        add_test(NAME fan_interaction COMMAND FanInteractionTests)
    endif()
endif()
