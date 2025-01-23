# build settings for SDL
set(SDL_TEST_LIBRARY OFF)
set(SDL_DISABLE_INSTALL ON)
set(SDL_DISABLE_UNINSTALL ON)
set(SDL_VULKAN ON)
set(SDL_DIRECTX OFF)
set(SDL_OPENG OFF)
set(SDL_OPENGLES OFF)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)

# fetch and build SDL
FetchContent_Declare(sdl
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG "preview-3.1.3"
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(sdl)

# link SDL to our renderer
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3-static)
