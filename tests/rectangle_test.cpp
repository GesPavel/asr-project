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

static std::pair<std::vector<asr::Vertex>, std::vector<unsigned int>> generate_rectangle_geometry_data(
                                                                          float width,float height,
                                                                          unsigned int width_segments_count,
                                                                          unsigned int height_segments_count
                                                                      )
{
    std::vector<asr::Vertex> vertices;
    std::vector<unsigned int> indices;

    float half_height{height * 0.5f};
    float segment_height{height / static_cast<float>(height_segments_count)};

    float half_width{width * 0.5f};
    float segment_width{width / static_cast<float>(width_segments_count)};

    for (unsigned int i = 0; i <= height_segments_count; ++i) {
        float y{static_cast<float>(i) * segment_height - half_height};
        for (unsigned int j = 0; j <= width_segments_count; ++j) {
            float x{static_cast<float>(j) * segment_width - half_width};
            vertices.push_back(asr::Vertex{
                x, y, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f
            });
        }
    }

    for (unsigned int i = 0; i < height_segments_count; ++i) {
        for (unsigned int j = 0; j < width_segments_count; ++j) {
            unsigned int index_a{i * (width_segments_count + 1) + j};
            unsigned int index_b{index_a + 1};
            unsigned int index_c{index_a + (width_segments_count + 1)};
            unsigned int index_d{index_c + 1};

            indices.push_back(index_a);
            indices.push_back(index_b);
            indices.push_back(index_c);

            indices.push_back(index_b);
            indices.push_back(index_d);
            indices.push_back(index_c);
        }
    }

    return std::make_pair(vertices, indices);
}

static std::pair<std::vector<asr::Vertex>, std::vector<unsigned int>> generate_rectangle_edges_data(
                                                                          float width,
                                                                          float height,
                                                                          unsigned int width_segments_count,
                                                                          unsigned int height_segments_count
                                                                      )
{
    std::vector<asr::Vertex> vertices;
    std::vector<unsigned int> indices;

    float half_height{height * 0.5f};
    float segment_height{height / static_cast<float>(height_segments_count)};

    float half_width{width * 0.5f};
    float segment_width{width / static_cast<float>(width_segments_count)};

    for (unsigned int i = 0; i <= height_segments_count; ++i) {
        float y{static_cast<float>(i) * segment_height - half_height};
        for (unsigned int j = 0; j <= width_segments_count; ++j) {
            float x{static_cast<float>(j) * segment_width - half_width};
            vertices.push_back(asr::Vertex{
                x, y, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f
            });
        }
    }

    for (unsigned int i = 0; i < height_segments_count; ++i) {
        for (unsigned int j = 0; j < width_segments_count; ++j) {
            unsigned int index_a{i * (width_segments_count + 1) + j};
            unsigned int index_b{index_a + 1};
            unsigned int index_c{index_a + (width_segments_count + 1)};
            unsigned int index_d{index_c + 1};

            indices.push_back(index_a); indices.push_back(index_b);
            indices.push_back(index_b); indices.push_back(index_c);
            indices.push_back(index_c); indices.push_back(index_a);

            indices.push_back(index_b); indices.push_back(index_d);
            indices.push_back(index_d); indices.push_back(index_c);
            indices.push_back(index_c); indices.push_back(index_b);
        }
    }

    return std::make_pair(vertices, indices);
}

static std::pair<std::vector<asr::Vertex>, std::vector<unsigned int>> generate_rectangle_vertices_data(
                                                                          float width,
                                                                          float height,
                                                                          unsigned int width_segments_count,
                                                                          unsigned int height_segments_count
                                                                      )
{
    std::vector<asr::Vertex> vertices;
    std::vector<unsigned int> indices;

    float half_height{height * 0.5f};
    float segment_height{height / static_cast<float>(height_segments_count)};

    float half_width{width * 0.5f};
    float segment_width{width / static_cast<float>(width_segments_count)};

    for (unsigned int i = 0; i <= height_segments_count; ++i) {
        float y{static_cast<float>(i) * segment_height - half_height};
        for (unsigned int j = 0; j <= width_segments_count; ++j) {
            float x{static_cast<float>(j) * segment_width - half_width};
            vertices.push_back(asr::Vertex{
                x, y, 0.0f,
                1.0f, 1.0f, 1.0f, 1.0f
            });
            indices.push_back(vertices.size() - 1);
        }
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
    auto [geometry_vertices, geometry_indices] = generate_rectangle_geometry_data(1.0f, 1.0f, 5, 5);
    auto gpu_geometry = generate_es2_gpu_geometry(
        GeometryType::Triangles,
        geometry_vertices,
        geometry_indices
    );
    auto [edge_vertices, edge_indices] = generate_rectangle_edges_data(1.0f, 1.0f, 5, 5);
    for (auto &vertex : edge_vertices) {
        vertex.z -= 0.01f; vertex.r = 1.0f; vertex.g = 0.7f; vertex.b = 0.7f;
    }
    auto gpu_edges_geometry = generate_es2_gpu_geometry(
        GeometryType::Lines,
        edge_vertices,
        edge_indices
    );
    auto [vertices, vertex_indices] = generate_rectangle_vertices_data(1.0f, 1.0f, 5, 5);
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
