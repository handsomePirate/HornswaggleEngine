#include "Binnacle_Render_Manager.hpp"
#include "RenderManager.hpp"

BINNACLE_API int *binnacle_init(const int width, const int height, const bool texture)
{
	render_manager *binnacle = new render_manager(false, 4, 3, 0, width, height, "OpenGL", !texture);
	binnacle_init_renderer(reinterpret_cast<int *>(binnacle));
	binnacle->set_camera(glm::vec3(), glm::vec3(), glm::vec3(0, 1, 0), 45, binnacle->get_aspect_ratio(), 1, 20);

	const auto shader_program_id = binnacle_create_shader_program(reinterpret_cast<int *>(binnacle), "vertex.glsl", "fragment.glsl");

	return reinterpret_cast<int *>(binnacle);
}

BINNACLE_API void binnacle_release(int *binnacle)
{
	delete reinterpret_cast<render_manager *>(binnacle);
}

BINNACLE_API bool binnacle_is_valid(int *binnacle)
{
	return reinterpret_cast<render_manager *>(binnacle)->is_valid();
}

BINNACLE_API float binnacle_get_aspect_ratio(int *binnacle)
{
	return reinterpret_cast<render_manager *>(binnacle)->get_aspect_ratio();
}

BINNACLE_API int binnacle_create_shader_program(int *binnacle, const std::string& vertex_shader_file, const std::string& fragment_shader_file,
								   const std::string& geometry_shader_file)
{
	return reinterpret_cast<render_manager *>(binnacle)->create_shader_program(vertex_shader_file, fragment_shader_file, geometry_shader_file);
}

BINNACLE_API void binnacle_init_renderer(int *binnacle)
{
	reinterpret_cast<render_manager *>(binnacle)->init_renderer();
}

BINNACLE_API void binnacle_load_model_data(int *binnacle, float *vertex_positions, const size_t vertex_count, unsigned int* indices, size_t const index_count,
								  const int mat_id)
{
	reinterpret_cast<render_manager *>(binnacle)->load_model_data(vertex_positions, vertex_count, indices, index_count, mat_id);
}

BINNACLE_API unsigned char *binnacle_render_to_texture(int *binnacle, const bool screen)
{
	return reinterpret_cast<render_manager *>(binnacle)->render_to_texture(screen);
}

BINNACLE_API void binnacle_dispose_texture(const unsigned char* pixels)
{
	delete[] pixels;
}

BINNACLE_API void binnacle_set_position(int *binnacle, const float x, const float y, const float z)
{
	reinterpret_cast<render_manager *>(binnacle)->get_camera().set_position(x, y, z);
}

BINNACLE_API void binnacle_set_focus(int *binnacle, const float x, const float y, const float z)
{
	reinterpret_cast<render_manager *>(binnacle)->get_camera().set_focus(x, y, z);
}

BINNACLE_API int binnacle_create_material(int *binnacle, const unsigned program, const std::string& filename_texture, const std::string& filename_normals)
{
	return reinterpret_cast<render_manager *>(binnacle)->create_material(program, filename_texture, filename_normals);
}

BINNACLE_API int binnacle_create_material_col(int *binnacle, const unsigned program, const float r, const float g, const float b, const std::string& filename_normals)
{
	return reinterpret_cast<render_manager *>(binnacle)->create_material(program, r, g, b, filename_normals);
}

BINNACLE_API void binnacle_delete_material(int *binnacle, const int index)
{
	reinterpret_cast<render_manager *>(binnacle)->delete_material(index);
}

BINNACLE_API void binnacle_update(int *binnacle)
{
	reinterpret_cast<render_manager *>(binnacle)->update();
}

BINNACLE_API void binnacle_render(int *binnacle)
{
	reinterpret_cast<render_manager *>(binnacle)->render();
}