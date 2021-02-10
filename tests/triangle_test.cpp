#include "asr.h"

#include <cmath>
#include <vector>

static const char Vertex_Shader_Source[] = R"(
    #version 110

    attribute vec4 position;
    attribute vec4 color;

    uniform float time;

    varying vec4 fragment_color;

    void main()
    {
        fragment_color = color;

        vec4 rotated_position = position;
        rotated_position.x = position.x * cos(time) - position.y * sin(time);
        rotated_position.y = position.x * sin(time) + position.y * cos(time);

        gl_Position = rotated_position;
    }
)";

static const char Fragment_Shader_Source[] = R"(
    #version 110

    varying vec4 fragment_color;

    void main()
    {
        gl_FragColor = fragment_color;
    }
)";

static const std::vector<asr::Vertex> Triangle_Geometry_Vertices = {
            //   Position             Color (RGBA)
    asr::Vertex{-0.5f, -0.305f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
    asr::Vertex{ 0.5f, -0.305f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f},
    asr::Vertex{ 0.0f,  0.565f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f}
};
static const std::vector<unsigned int> Triangle_Geometry_Indices = {
    0, 1, 2
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_es2_sdl_window();
    create_es2_shader_program(
        Vertex_Shader_Source,
        Fragment_Shader_Source
    );

    std::vector<Vertex> vertices{Triangle_Geometry_Vertices};
    auto gpu_geometry = generate_es2_gpu_geometry(
        GeometryType::Triangles,
        Triangle_Geometry_Vertices,
        Triangle_Geometry_Indices
    );

    prepare_for_es2_rendering();

    bool should_stop = false;
    while (!should_stop) {
        process_es2_sdl_window_events(&should_stop);

        prepare_to_render_es2_frame();

        set_es2_gpu_geometry_current(&gpu_geometry);
        render_current_es2_gpu_geometry();

        finish_es2_frame_rendering();
    }

    destroy_es2_gpu_geometry(gpu_geometry);
    destroy_es2_shader_program();
    destroy_es2_sdl_window();

    return 0;
}
