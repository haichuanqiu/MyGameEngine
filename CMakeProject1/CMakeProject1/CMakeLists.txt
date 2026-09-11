cmake_minimum_required(VERSION 3.10)

project(CMakeProject1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# =========================
# GLFW
# =========================

add_subdirectory(external/glfw)


# =========================
# ImGui
# =========================

set(IMGUI_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
)

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)


# =========================
# GLAD
# =========================

set(GLAD_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c
)


# =========================
# Your source files
# =========================

file(GLOB_RECURSE PROJECT_SRC
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)


# =========================
# Executable
# =========================

add_executable(CMakeProject1
    ${PROJECT_SRC}
    ${IMGUI_SRC}
    ${GLAD_SRC})


# =========================
# Include directories
# =========================

target_include_directories(CMakeProject1 PRIVATE

    ${CMAKE_CURRENT_SOURCE_DIR}/src

    ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include

    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends

    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
)


# =========================
# Libraries
# =========================

target_link_libraries(CMakeProject1
    glfw
    opengl32
)

target_compile_definitions(CMakeProject1 PRIVATE
    GLM_ENABLE_EXPERIMENTAL
)
# =========================
# Windows
# =========================

if(WIN32)

    target_compile_definitions(CMakeProject1 PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD
    )

endif()