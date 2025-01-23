# fetch and build SDL
FetchContent_Declare(stb
    GIT_REPOSITORY "https://github.com/nothings/stb.git"
    GIT_TAG "master"
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(stb)
target_include_directories(${PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR})