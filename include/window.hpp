#pragma once
#include <string>
#include <glbinding/gl46core/gl.h>
#include "glbinding/AbstractFunction.h"
#include <glbinding/glbinding.h>
#include <SDL3/SDL.h>
#include <fmt/base.h>
using namespace gl46core;

struct Window {
    void init(int width, int height, std::string name) {
        // init the SDL video subsystem before anything else
        bool res = SDL_InitSubSystem(SDL_INIT_VIDEO);
        if (!res) fmt::println("{}", SDL_GetError());

        // OpenGL context settings
        SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_CONTEXT_PROFILE_MASK, SDL_GLprofile::SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GLattr::SDL_GL_CONTEXT_FLAGS, SDL_GLcontextFlag::SDL_GL_CONTEXT_DEBUG_FLAG);

        // create a window specifically with OpenGL support
        _window_p = SDL_CreateWindow(name.c_str(), width, height, SDL_WINDOW_OPENGL);
        if (_window_p == nullptr) fmt::println("{}", SDL_GetError());

        // create opengl context
        _context = SDL_GL_CreateContext(_window_p);
        if (_context == nullptr) fmt::println("{}", SDL_GetError());

        // lazy loader for OpenGL functions
        glbinding::initialize(SDL_GL_GetProcAddress);
        // enable error logging
        glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue, { "glGetError" });
        glbinding::setAfterCallback([](const glbinding::FunctionCall& call) {
            static unsigned int callCount = 0;
            static constexpr unsigned int maxCalls = 3;

            const auto error_code = glGetError();
            if (error_code != GL_NO_ERROR)
            {
                // only allow a certain number of errors to be printed
                if (++callCount > maxCalls) {
                    if (callCount == maxCalls) fmt::println("Too many OpenGL errors");
                    return;
                }

                // print out the function name, parameters and return value
                fmt::print("{}(", call.function->name());
                for (unsigned i = 0; i < call.parameters.size(); ++i) {
                    fmt::print("{}", *(uint32_t*)call.parameters[i].get());
                    if (i < call.parameters.size() - 1) fmt::print(", ");
                }
                fmt::print(")");
                if (call.returnValue) fmt::print(" -> {}", *(uint32_t*)call.returnValue.get());
                fmt::println("");

                // print out error code
                std::string error;
                switch (error_code) {
                    case GL_INVALID_ENUM:                  fmt::println("Error: {}", "INVALID_ENUM"); break;
                    case GL_INVALID_VALUE:                 fmt::println("Error: {}", "INVALID_VALUE"); break;
                    case GL_INVALID_OPERATION:             fmt::println("Error: {}", "INVALID_OPERATION"); break;
                    case GL_STACK_OVERFLOW:                fmt::println("Error: {}", "STACK_OVERFLOW"); break;
                    case GL_STACK_UNDERFLOW:               fmt::println("Error: {}", "STACK_UNDERFLOW"); break;
                    case GL_OUT_OF_MEMORY:                 fmt::println("Error: {}", "OUT_OF_MEMORY"); break;
                    case GL_INVALID_FRAMEBUFFER_OPERATION: fmt::println("Error: {}", "INVALID_FRAMEBUFFER_OPERATION"); break;
                    default:                               fmt::println("Error: {}", "Unknown error"); break;
                }
            }
        });

        // glEnable(GL_CULL_FACE); // cull backfaces
        glEnable(GL_DEPTH_TEST); // enable depth buffer and depth testing
        // glEnable(GL_FRAMEBUFFER_SRGB); // gamma corrected framebuffer
        SDL_GL_SetSwapInterval(1);
    }
    void destroy() {
        SDL_Quit();
    }

    SDL_Window* _window_p;
    SDL_GLContext _context;
};