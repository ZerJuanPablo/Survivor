FetchContent_Declare(imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG "v1.91.5"
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(imgui)
target_include_directories(${PROJECT_NAME} PRIVATE
"${imgui_SOURCE_DIR}/"
"${imgui_SOURCE_DIR}/backends")
target_sources(${PROJECT_NAME} PRIVATE
"${imgui_SOURCE_DIR}/imgui.cpp"
"${imgui_SOURCE_DIR}/imgui_draw.cpp"
"${imgui_SOURCE_DIR}/imgui_demo.cpp"
"${imgui_SOURCE_DIR}/imgui_tables.cpp"
"${imgui_SOURCE_DIR}/imgui_widgets.cpp"
"${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
"${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")