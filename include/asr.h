#ifndef AUR_H
#define AUR_H

#include <GL/glew.h>
#include <SDL.h>

#include <iostream>
#include <chrono>

/*
 * Platform Quirks
 */

#ifdef __APPLE__
#define glGenVertexArrays glGenVertexArraysAPPLE
#define glBindVertexArray glBindVertexArrayAPPLE
#define glDeleteVertexArrays glDeleteVertexArraysAPPLE
#endif

namespace asr
{
    /*
     * SDL Window Globals
     */

    static int windows_width{500};
    static int windows_height{500};

    static SDL_Window *window{nullptr};
    static SDL_GLContext gl_context;

    /*
     * Shader Identifiers
     */

    static GLuint shader_program{0};
    static GLint position_attribute_location{-1};
    static GLint color_attribute_location{-1};
    static GLint time_uniform_location{-1};

    /*
     * Geometry Buffers' Data
     */

    enum GeometryType
    {
        Points,
        Lines,
        LineLoop,
        LineStrip,
        Triangles,
        TriangleFan,
        TriangleStrip
    };

    static GLuint vertex_array_object{0};
    static GLuint vertex_buffer_object{0};

    static GLenum geometry_type;
    static size_t geometry_vertex_count{0};

    /*
     * Utility Data
     */

    static std::chrono::system_clock::time_point rendering_start_time;

    /*
     * SDL Window Handling
     */

    static void create_es2_sdl_window()
    {
        SDL_Init(SDL_INIT_VIDEO);
        window =
            SDL_CreateWindow(
                "ASR: First Triangle Test",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                windows_width, windows_height,
                SDL_WINDOW_OPENGL
            );
        SDL_GL_GetDrawableSize(window, reinterpret_cast<int *>(&windows_width), reinterpret_cast<int *>(&windows_height));
        gl_context = SDL_GL_CreateContext(window);
        glewExperimental = GL_TRUE; glewInit();
        SDL_GL_SetSwapInterval(1);
    }

    static void process_es2_sdl_window_events(bool *should_stop)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                *should_stop = true;
            }
        }
    }

    static void destroy_es2_sdl_window()
    {
        SDL_GL_DeleteContext(gl_context);

        SDL_DestroyWindow(window);
        window = nullptr;

        SDL_Quit();
    }

    /*
     * Shader Program Handling
     */

    static void create_es2_shader_program(const char *vertex_shader_source, const char *fragment_shader_source)
    {
        GLint status;

        GLuint vertex_shader_object = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader_object, 1, static_cast<const GLchar **>(&vertex_shader_source), nullptr);
        glCompileShader(vertex_shader_object);
        glGetShaderiv(vertex_shader_object, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            GLint info_log_length;
            glGetShaderiv(vertex_shader_object, GL_INFO_LOG_LENGTH, &info_log_length);
            if (info_log_length > 0) {
                auto *info_log = new GLchar[static_cast<size_t>(info_log_length)];

                glGetShaderInfoLog(vertex_shader_object, info_log_length, nullptr, info_log);
                std::cerr << "Failed to compile a vertex shader" << std::endl
                          << "Compilation log:\n" << info_log << std::endl << std::endl;

                delete[] info_log;
            }
        }

        GLuint fragment_shader_object = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader_object, 1, static_cast<const GLchar **>(&fragment_shader_source), nullptr);
        glCompileShader(fragment_shader_object);
        glGetShaderiv(fragment_shader_object, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            GLint info_log_length;
            glGetShaderiv(fragment_shader_object, GL_INFO_LOG_LENGTH, &info_log_length);
            if (info_log_length > 0) {
                auto *info_log = new GLchar[static_cast<size_t>(info_log_length)];

                glGetShaderInfoLog(fragment_shader_object, info_log_length, nullptr, info_log);
                std::cerr << "Failed to compile a fragment shader" << std::endl
                          << "Compilation log:\n" << info_log << std::endl;

                delete[] info_log;
            }
        }

        shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader_object);
        glAttachShader(shader_program, fragment_shader_object);
        glLinkProgram(shader_program);
        glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            GLint info_log_length;
            glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &info_log_length);
            if (info_log_length > 0) {
                auto *info_log = new GLchar[static_cast<size_t>(info_log_length)];

                glGetProgramInfoLog(shader_program, info_log_length, nullptr, info_log);
                std::cerr << "Failed to link a shader program" << std::endl
                          << "Linker log:\n" << info_log << std::endl;

                delete[] info_log;
            }
        }

        glDetachShader(shader_program, vertex_shader_object);
        glDetachShader(shader_program, fragment_shader_object);
        glDeleteShader(vertex_shader_object);
        glDeleteShader(fragment_shader_object);

        position_attribute_location = glGetAttribLocation(shader_program, "position");
        color_attribute_location = glGetAttribLocation(shader_program, "color");

        time_uniform_location = glGetUniformLocation(shader_program, "time");
    }

    static void destroy_es2_shader_program()
    {
        glUseProgram(0);
        glDeleteProgram(shader_program);
        shader_program = 0;

        position_attribute_location = -1;
        color_attribute_location = -1;
        time_uniform_location = -1;
    }

    /*
     * Geometry Buffer Handling
     */

    static GLenum convert_geometry_type_to_es2_geometry_type(GeometryType type)
    {
        switch (type) {
            case GeometryType::Points:
                return GL_POINTS;
            case GeometryType::Lines:
                return GL_LINES;
            case GeometryType::LineLoop:
                return GL_LINE_LOOP;
            case GeometryType::LineStrip:
                return GL_LINE_STRIP;
            case GeometryType::Triangles:
                return GL_TRIANGLES;
            case GeometryType::TriangleFan:
                return GL_TRIANGLE_FAN;
            case GeometryType::TriangleStrip:
                return GL_TRIANGLE_STRIP;
        }

        return GL_TRIANGLES;
    }

    static void generate_es2_geometry(GeometryType type, const float *data, const size_t vertex_count)
    {
        geometry_vertex_count = vertex_count;
        geometry_type = convert_geometry_type_to_es2_geometry_type(type);

        glGenVertexArrays(1, &vertex_array_object);
        glBindVertexArray(vertex_array_object);

        glGenBuffers(1, &vertex_buffer_object);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
        glBufferData(
            GL_ARRAY_BUFFER,
            geometry_vertex_count * 7 * sizeof(float), data,
            GL_STATIC_DRAW
        );

        GLsizei stride = sizeof(GLfloat) * 7;
        glEnableVertexAttribArray(position_attribute_location);
        glVertexAttribPointer(
            position_attribute_location,
            3, GL_FLOAT, GL_FALSE, stride, static_cast<const GLvoid *>(nullptr)
        );
        glEnableVertexAttribArray(color_attribute_location);
        glVertexAttribPointer(
            color_attribute_location,
            4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(sizeof(GLfloat) * 3)
        );

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    static void destroy_es2_geometry()
    {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vertex_array_object);
        vertex_array_object = 0;

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vertex_buffer_object);
        vertex_buffer_object = 0;
    }

    /*
     * Rendering
     */

    static void prepare_for_es2_rendering()
    {
        glClearColor(0, 0, 0, 0);
        glViewport(0, 0, static_cast<GLsizei>(windows_width), static_cast<GLsizei>(windows_height));

        rendering_start_time = std::chrono::system_clock::now();
    }

    static void render_next_es2_frame()
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        glBindVertexArray(vertex_array_object);

        if (time_uniform_location != -1) {
            double time =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now() - rendering_start_time
                ).count();

            glUniform1f(time_uniform_location, time / 1000.0);
        }

        glDrawArrays(geometry_type, 0, geometry_vertex_count);

        SDL_GL_SwapWindow(window);
    }
}
#endif
