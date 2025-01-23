# find OpenGL headers (usually included with graphics drivers)
find_package(OpenGL REQUIRED COMPONENTS)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# glbinding build options
set(OPTION_BUILD_TOOLS OFF)
set(OPTION_BUILD_EXAMPLES OFF)
set(OPTION_USE_GIT_INFORMATION OFF)

# fetch and build OpenGL function loader
FetchContent_Declare(glbinding
    GIT_REPOSITORY "https://github.com/cginternals/glbinding.git"
    GIT_TAG "v3.3.0"
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(glbinding)
target_link_libraries(${PROJECT_NAME} PRIVATE glbinding::glbinding)