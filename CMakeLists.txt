cmake_minimum_required(VERSION 3.20)
project(Project3_CMAKE VERSION 1.0)

# 設定 C++17 標準
set(CMAKE_CXX_STANDARD 17)

# 強制要求編譯器支援此標準（若不支援則報錯，防止降級到舊版本）
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 禁用編譯器特定擴充（如 gnu++17），僅使用標準 C++（-std=c++17）
set(CMAKE_CXX_EXTENSIONS OFF)

# ─── Collect Sources (Only from src/) ───────────────────────────
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# ─── ImGui Sources ──────────────────────────────────────────────
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/include/imgui")
set(IMGUI_SRC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

# ─── Executable ─────────────────────────────────────────────────
add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${IMGUI_SRC}
)

# ─── Include Directories ────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/include/imgui"
    "${CMAKE_SOURCE_DIR}/include/imgui/backends"
)

# ─── Compile Definitions ────────────────────────────────────────
# OpenMesh 在 Windows 上需要 _USE_MATH_DEFINES 才能取得 M_PI 等常數
target_compile_definitions(${PROJECT_NAME} PRIVATE
    _USE_MATH_DEFINES
)

# ─── Debug Symbols (MSVC) ───────────────────────────────────────
# CMake 4.x 不再自動將 /Zi 加入 MSVC Debug 旗標，須明確指定
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Zi>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/DEBUG:FULL>
    )
endif()

# ─── Link .lib Files ────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    "${CMAKE_SOURCE_DIR}/lib/glew/glew32.lib"
    "${CMAKE_SOURCE_DIR}/lib/GLFW/glfw3.lib"
    debug     "${CMAKE_SOURCE_DIR}/lib/OpenMesh/OpenMeshCored.lib"
    optimized "${CMAKE_SOURCE_DIR}/lib/OpenMesh/OpenMeshCore.lib"
    debug     "${CMAKE_SOURCE_DIR}/lib/OpenMesh/OpenMeshToolsd.lib"
    optimized "${CMAKE_SOURCE_DIR}/lib/OpenMesh/OpenMeshTools.lib"
)

# ─── Copy .dll Files to Output Directory ────────────────────────
file(GLOB DLL_FILES "${CMAKE_SOURCE_DIR}/dll/*.dll")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL_FILES}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "複製 DLLs 到輸出目錄..."
)

# ─── Copy res/ to Output Directory ──────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/res"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
    COMMENT "複製 RES 到輸出目錄..."
)

