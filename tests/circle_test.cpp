#include "asr.h"

#include <cmath>
#include <vector>
#include <utility>

static const char Vertex_Shader_Source[] = R"(
    #version 110

    attribute vec4 position;
    attribute vec4 color;

    uniform mat4 model_view_projection_matrix;

    varying vec4 fragment_color;

    void main()
    {
        fragment_color = color;

        gl_Position = model_view_projection_matrix * position;
        gl_PointSize = 10.0;
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

static std::pair<std::vector<asr::Vertex>, std::vector<unsigned int>> generate_circle_geometry_data(
                                                                          float radius,
                                                                          unsigned int segment_count
                                                                      )
{
    std::vector<asr::Vertex> vertices;
    std::vector<unsigned int> indices;

    float angle{0.0f};
    float angle_delta{2.0f * static_cast<float>(M_PI) / static_cast<float>(segment_count)};

    vertices.push_back(asr::Vertex{
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    });
    float x{cosf(angle) * radius};
    float y{sinf(angle) * radius};
    vertices.push_back(asr::Vertex{
        x, y, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    });

    for (unsigned int i = 0; i < segment_count; ++i) {
        indices.push_back(0);
        indices.push_back(vertices.size() - 1);

        float next_x{cosf(angle + angle_delta) * radius};
        float next_y{sinf(angle + angle_delta) * radius};
        vertices.push_back(asr::Vertex{
            next_x, next_y, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        });
        indices.push_back(vertices.size() - 1);

        angle += angle_delta;
    }

    return std::make_pair(vertices, indices);
}

static std::pair<std::vector<asr::Vertex>, std::vector<unsigned int>> generate_circle_edges_data(
                                                                          float radius,
                                                                          unsigned int segment_count
                                                                      )
{
    std::vector<asr::Vertex> vertices;
    std::vector<unsigned int> indices;

    float angle{0.0f};
    float angle_delta{2.0f * static_cast<float>(M_PI) / static_cast<float>(segment_count)};

    vertices.push_back(asr::Vertex{
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    });
    float x{cosf(angle) * radius};
    float y{sinf(angle) * radius};
    vertices.push_back(asr::Vertex{
        x, y, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    });

    for (unsigned int i = 0; i < segment_count; ++i) {
        indices.push_back(0);
        indices.push_back(vertices.size() - 1);
        indices.push_back(vertices.size() - 1);

        float next_x{cosf(angle + angle_delta) * radius};
        float next_y{sinf(angle + angle_delta) * radius};
        vertices.push_back(asr::Vertex{
            next_x, next_y, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        });
        indices.push_back(vertices.size() - 1);

        angle += angle_delta;
    }

    return std::make_pair(vertices, indices);
}

static std::pair<std::vector<asr::Vertex>, std::vector<unsigned int>> generate_circle_vertices_data(
                                                                          float radius,
                                                                          unsigned int segment_count
                                                                      )
{
    std::vector<asr::Vertex> vertices;
    std::vector<unsigned int> indices;

    float angle{0.0f};
    float angle_delta{2.0f * static_cast<float>(M_PI) / static_cast<float>(segment_count)};

    vertices.push_back(asr::Vertex{
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    });
    indices.push_back(0);
    float x{cosf(angle) * radius};
    float y{sinf(angle) * radius};
    vertices.push_back(asr::Vertex{
        x, y, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    });
    indices.push_back(1);

    for (unsigned int i = 0; i < segment_count; ++i) {
        float next_x{cosf(angle + angle_delta) * radius};
        float next_y{sinf(angle + angle_delta) * radius};
        vertices.push_back(asr::Vertex{
            next_x, next_y, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        });
        indices.push_back(vertices.size() - 1);

        angle += angle_delta;
    }

    return std::make_pair(vertices, indices);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_es2_sdl_window();

    create_es2_shader_program(
        Vertex_Shader_Source,
        Fragment_Shader_Source
    );
    auto [geometry_vertices, geometry_indices] = generate_circle_geometry_data(0.5f, 10);
    auto gpu_geometry = generate_es2_gpu_geometry(
        GeometryType::Triangles,
        geometry_vertices,
        geometry_indices
    );
    auto [edge_vertices, edge_indices] = generate_circle_edges_data(0.5f, 10);
    for (auto &vertex : edge_vertices) {
        vertex.z -= 0.01f; vertex.r = 1.0f; vertex.g = 0.7f; vertex.b = 0.7f;
    }
    auto gpu_edges_geometry = generate_es2_gpu_geometry(
        GeometryType::Lines,
        edge_vertices,
        edge_indices
    );
    auto [vertices, vertex_indices] = generate_circle_vertices_data(0.5f, 10);
    for (auto &vertex : vertices) {
        vertex.z -= 0.02f; vertex.r = 1.0f; vertex.g = 0.0f; vertex.b = 0.0f;
    }
    auto gpu_vertices_geometry = generate_es2_gpu_geometry(
        GeometryType::Points,
        vertices,
        vertex_indices
    );

    prepare_for_es2_rendering();
    set_es2_line_width(3);

    bool should_stop = false;
    while (!should_stop) {
        process_es2_sdl_window_events(&should_stop);

        prepare_to_render_es2_frame();

        set_es2_gpu_geometry_current(&gpu_geometry);
        render_current_es2_gpu_geometry();

        set_es2_gpu_geometry_current(&gpu_edges_geometry);
        render_current_es2_gpu_geometry();

        set_es2_gpu_geometry_current(&gpu_vertices_geometry);
        render_current_es2_gpu_geometry();

        finish_es2_frame_rendering();
    }

    destroy_es2_gpu_geometry(gpu_geometry);
    destroy_es2_gpu_geometry(gpu_edges_geometry);
    destroy_es2_gpu_geometry(gpu_vertices_geometry);
    destroy_es2_shader_program();

    destroy_es2_sdl_window();

    return 0;
}
