#include "asr.h"

#include <cmath>
#include <utility>
#include <vector>

static const char Vertex_Shader_Source[] = R"(
    #version 110

    attribute vec4 position;
    attribute vec4 color;
    attribute vec4 texture_coordinates;

    uniform bool texture_enabled;
    uniform mat4 texture_transformation_matrix;

    uniform mat4 model_view_projection_matrix;

    varying vec4 fragment_color;
    varying vec2 fragment_texture_coordinates;

    void main()
    {
        fragment_color = color;
        if (texture_enabled) {
            vec4 transformed_texture_coordinates = texture_transformation_matrix * vec4(texture_coordinates.st, 0.0, 1.0);
            fragment_texture_coordinates = vec2(transformed_texture_coordinates);
        }

        gl_Position = model_view_projection_matrix * position;
        gl_PointSize = 10.0;
    }
)";

static const char Fragment_Shader_Source[] = R"(
    #version 110

    #define TEXTURING_MODE_ADDITION            0
    #define TEXTURING_MODE_SUBTRACTION         1
    #define TEXTURING_MODE_REVERSE_SUBTRACTION 2
    #define TEXTURING_MODE_MODULATION          3
    #define TEXTURING_MODE_DECALING            4

    uniform bool texture_enabled;
    uniform int texturing_mode;
    uniform sampler2D texture_sampler;

    varying vec4 fragment_color;
    varying vec2 fragment_texture_coordinates;

    void main()
    {
        gl_FragColor = fragment_color;

        if (texture_enabled) {
            if (texturing_mode == TEXTURING_MODE_ADDITION) {
                gl_FragColor += texture2D(texture_sampler, fragment_texture_coordinates);
            } else if (texturing_mode == TEXTURING_MODE_MODULATION) {
                gl_FragColor *= texture2D(texture_sampler, fragment_texture_coordinates);
            } else if (texturing_mode == TEXTURING_MODE_DECALING) {
                vec4 texel_color = texture2D(texture_sampler, fragment_texture_coordinates);
                gl_FragColor.rgb = mix(gl_FragColor.rgb, texel_color.rgb, texel_color.a);
            } else if (texturing_mode == TEXTURING_MODE_SUBTRACTION) {
                gl_FragColor -= texture2D(texture_sampler, fragment_texture_coordinates);
            } else if (texturing_mode == TEXTURING_MODE_REVERSE_SUBTRACTION) {
                gl_FragColor = texture2D(texture_sampler, fragment_texture_coordinates) - gl_FragColor;
            }
        }
    }
)";

static std::pair<std::vector<asr::Vertex>, std::vector<unsigned int>> generate_sphere_geometry_data(
                                                                          float radius,
                                                                          unsigned int width_segments_count,
                                                                          unsigned int height_segments_count
                                                                      )
{
    std::vector<asr::Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int ring = 0; ring <= height_segments_count; ++ring) {
        float v{static_cast<float>(ring) / static_cast<float>(height_segments_count)};
        float phi{v * asr::pi};

        for (unsigned int segment = 0; segment <= width_segments_count; ++segment) {
            float u{static_cast<float>(segment) / static_cast<float>(width_segments_count)};
            float theta{u * asr::two_pi};

            float cos_phi{std::cosf(phi)};
            float sin_phi{std::sinf(phi)};
            float cos_theta{std::cosf(theta)};
            float sin_theta{std::sinf(theta)};

            float y{cos_phi * radius};
            float x{sin_phi * cos_theta * radius};
            float z{sin_phi * sin_theta * radius};

            vertices.push_back(asr::Vertex{
                x, y, z,
                1.0f, 1.0f, 1.0f, 1.0f,
                1.0f - u, v
            });
        }
    }

    for (unsigned int ring = 0; ring < height_segments_count; ++ring) {
        for (unsigned int segment = 0; segment < width_segments_count; ++segment) {
            unsigned int index_a{ring * (width_segments_count + 1) + segment};
            unsigned int index_b{index_a + 1};

            unsigned int index_c{index_a + (width_segments_count + 1)};
            unsigned int index_d{index_c + 1};

            if (ring != 0) {
                indices.push_back(index_a);
                indices.push_back(index_b);
                indices.push_back(index_c);
            }

            if (ring != height_segments_count - 1) {
                indices.push_back(index_b);
                indices.push_back(index_d);
                indices.push_back(index_c);
            }
        }
    }

    return std::make_pair(vertices, indices);
}

static std::pair<std::vector<asr::Vertex>, std::vector<unsigned int>> generate_sphere_edges_data(
                                                                          float radius,
                                                                          unsigned int width_segments_count,
                                                                          unsigned int height_segments_count
                                                                      )
{
    std::vector<asr::Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int ring = 0; ring <= height_segments_count; ++ring) {
        float v{static_cast<float>(ring) / static_cast<float>(height_segments_count)};
        for (unsigned int segment = 0; segment <= width_segments_count; ++segment) {
            float u{static_cast<float>(segment) / static_cast<float>(width_segments_count)};

            float theta{u * asr::two_pi};
            float phi{v * asr::pi};

            float cos_theta{std::cosf(theta)};
            float sin_theta{std::sinf(theta)};
            float sin_phi{std::sinf(phi)};
            float cos_phi{std::cosf(phi)};

            float x{cos_theta * sin_phi * radius};
            float y{cos_phi * radius};
            float z{sin_theta * sin_phi * radius};

            vertices.push_back(asr::Vertex{
                x, y, z,
                1.0f, 1.0f, 1.0f, 1.0f,
                1.0f - u, v
            });
        }
    }

    for (unsigned int ring = 0; ring < height_segments_count; ++ring) {
        for (unsigned int segment = 0; segment < width_segments_count; ++segment) {
            unsigned int index_a{ring * (width_segments_count + 1) + segment};
            unsigned int index_b{index_a + 1};

            unsigned int index_c{index_a + (width_segments_count + 1)};
            unsigned int index_d{index_c + 1};

            if (ring != 0) {
                indices.push_back(index_a); indices.push_back(index_b);
                indices.push_back(index_b); indices.push_back(index_c);
                indices.push_back(index_c); indices.push_back(index_a);
            }

            if (ring != height_segments_count - 1) {
                indices.push_back(index_b); indices.push_back(index_d);
                indices.push_back(index_d); indices.push_back(index_c);
                indices.push_back(index_c); indices.push_back(index_b);
            }
        }
    }

    return std::make_pair(vertices, indices);
}

static std::pair<std::vector<asr::Vertex>, std::vector<unsigned int>> generate_sphere_vertices_data(
                                                                          float radius,
                                                                          unsigned int width_segments_count,
                                                                          unsigned int height_segments_count
                                                                      )
{
    std::vector<asr::Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int ring = 0; ring <= height_segments_count; ++ring) {
        float v{static_cast<float>(ring) / static_cast<float>(height_segments_count)};
        for (unsigned int segment = 0; segment <= width_segments_count; ++segment) {
            float u{static_cast<float>(segment) / static_cast<float>(width_segments_count)};

            float theta{u * asr::two_pi};
            float phi{v * asr::pi};

            float cos_theta{std::cosf(theta)};
            float sin_theta{std::sinf(theta)};
            float sin_phi{std::sinf(phi)};
            float cos_phi{std::cosf(phi)};

            float x{cos_theta * sin_phi * radius};
            float y{cos_phi * radius};
            float z{sin_theta * sin_phi * radius};

            vertices.push_back(asr::Vertex{
                x, y, z,
                1.0f, 1.0f, 1.0f, 1.0f,
                1.0f - u, v
            });
            indices.push_back(vertices.size() - 1);
        }
    }

    return std::make_pair(vertices, indices);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    using namespace asr;

    create_window(500, 500);

    create_shader_program(
        Vertex_Shader_Source,
        Fragment_Shader_Source
    );
    auto [geometry_vertices, geometry_indices] = generate_sphere_geometry_data(0.5f, 20, 20);
    auto geometry = generate_geometry(
        GeometryType::Triangles,
        geometry_vertices,
        geometry_indices
    );
    auto [edge_vertices, edge_indices] = generate_sphere_edges_data(0.501f, 20, 20);
    auto edges_geometry = generate_geometry(
        GeometryType::Lines,
        edge_vertices,
        edge_indices
    );
    auto [vertices, vertex_indices]  = generate_sphere_vertices_data(0.502f, 20, 20);
    for (auto &vertex : vertices) {
        vertex.r = 1.0f; vertex.g = 0.0f; vertex.b = 0.0f;
    }
    auto vertices_geometry = generate_geometry(
        GeometryType::Points,
        vertices,
        vertex_indices
    );
    auto image = read_image_file("data/images/uv_test.png");
    auto texture = generate_texture(image);

    prepare_for_rendering();

    set_line_width(3);
    enable_depth_test();
    enable_face_culling();

    static const float CAMERA_SPEED{6.0f};
    static const float CAMERA_ROT_SPEED{1.5f};
    static const float CAMERA_FOV{1.13f};
    static const float CAMERA_NEAR_PLANE{0.1f};
    static const float CAMERA_FAR_PLANE{100.0f};

    glm::vec3 camera_position{0.0f, 0.0f, 1.5f};
    glm::vec3 camera_rotation{0.0f, 0.0f, 0.0f};
    set_keys_down_event_handler([&](const uint8_t *keys) {
        if (keys[SDL_SCANCODE_ESCAPE]) std::exit(0);
        if (keys[SDL_SCANCODE_W]) camera_rotation.x -= CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_A]) camera_rotation.y += CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_S]) camera_rotation.x += CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_D]) camera_rotation.y -= CAMERA_ROT_SPEED * get_dt();
        if (keys[SDL_SCANCODE_UP]) {
            glm::vec3 shift = (get_view_matrix() * glm::vec4{0.0f, 0.0f, 1.0f, 0.0f} * (CAMERA_SPEED * get_dt())).xyz();
            camera_position -= shift;
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            glm::vec3 shift = (get_view_matrix() * glm::vec4{0.0f, 0.0f, 1.0f, 0.0f} * (CAMERA_SPEED * get_dt())).xyz();
            camera_position += shift;
        }
    });
    set_matrix_mode(MatrixMode::Projection);
    load_perspective_projection_matrix(CAMERA_FOV, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);

    bool should_stop{false};
    while (!should_stop) {
        process_window_events(&should_stop);

        prepare_to_render_frame();

        set_matrix_mode(MatrixMode::View);
        load_identity_matrix();
        translate_matrix(camera_position);
        rotate_matrix(camera_rotation);

        set_texture_current(&texture);
        set_geometry_current(&geometry);
        render_current_geometry();

        set_texture_current(nullptr);
        set_geometry_current(&edges_geometry);
        render_current_geometry();
        set_geometry_current(&vertices_geometry);
        render_current_geometry();

        finish_frame_rendering();
    }

    destroy_texture(texture);
    destroy_geometry(geometry);
    destroy_geometry(edges_geometry);
    destroy_geometry(vertices_geometry);
    destroy_shader_program();

    destroy_window();

    return 0;
}
