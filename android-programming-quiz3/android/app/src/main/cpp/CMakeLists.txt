cmake_minimum_required(VERSION 3.22.1)

project(FanDashboardHUDAndroid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../..")
set(SCENE_DIR "${PROJECT_ROOT}/Scene")
set(CMAKE_MODULE_PATH "${PROJECT_ROOT}/cmake")
include(ImportDependencies)
import_dependencies()

add_library(fan3d SHARED
    NativeTemplate.cpp
    ${SCENE_DIR}/Fan.cpp
    ${SCENE_DIR}/Ground.cpp
    ${SCENE_DIR}/Renderer.cpp
    ${SCENE_DIR}/Scene3D.cpp
    ${SCENE_DIR}/SceneHUD.cpp
    ${SCENE_DIR}/Transform.cpp
)

target_include_directories(fan3d PRIVATE ${SCENE_DIR})
target_compile_definitions(fan3d PRIVATE PLATFORM_ANDROID)
target_compile_options(fan3d PRIVATE -Wall -Wextra -Wpedantic)
set_source_files_properties(${SCENE_DIR}/Transform.cpp PROPERTIES
    COMPILE_OPTIONS "-Wno-unused-parameter"
)

find_library(android-lib android REQUIRED)
find_library(log-lib log REQUIRED)
find_library(glesv3-lib GLESv3 REQUIRED)
find_library(egl-lib EGL REQUIRED)

target_link_libraries(fan3d
    glm::glm
    ${android-lib}
    ${log-lib}
    ${glesv3-lib}
    ${egl-lib}
)
