#include "asr.h"

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
    //           Position             Color (RGBA)            Texture Coordinates (UV)
    asr::Vertex{ 0.5f,   0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  0.5f },
    asr::Vertex{-0.25f,  0.43f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.25f, 0.07f},
    asr::Vertex{-0.25f, -0.43f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.25f, 0.93f}
};
static const std::vector<unsigned int> Triangle_Geometry_Indices = { 0, 1, 2 };

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(500, 500);

    create_shader_program(
        Vertex_Shader_Source,
        Fragment_Shader_Source
    );
    auto geometry = generate_geometry(
        GeometryType::Triangles,
        Triangle_Geometry_Vertices,
        Triangle_Geometry_Indices
    );

    prepare_for_rendering();

    bool should_stop{false};
    while (!should_stop) {
        process_window_events(&should_stop);

        prepare_to_render_frame();

        set_geometry_current(&geometry);
        render_current_geometry();

        finish_frame_rendering();
    }

    destroy_geometry(geometry);
    destroy_shader_program();

    destroy_window();

    return 0;
}
