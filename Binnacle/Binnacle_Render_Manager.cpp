#include "Binnacle_Render_Manager.hpp"
#include "RenderManager.hpp"

BINNACLE_API void binnacle_init(const int width, const int height, const bool texture)
{
	rm = new render_manager(false, 4, 3, 0, width, height, "OpenGL", !texture);
	binnacle_init_renderer();
	rm->set_camera(glm::vec3(), glm::vec3(), glm::vec3(0, 1, 0), 45, rm->get_aspect_ratio(), 1, 20);

	const auto shader_program_id = binnacle_create_shader_program("vertex.glsl", "fragment.glsl");
	const auto mat_id = rm->create_material(shader_program_id, glm::vec3(0.0f, 0.2f, 0.9f));

	if (mat_id != 0) return;
}

BINNACLE_API void binnacle_release()
{
	delete rm;
}

BINNACLE_API bool binnacle_is_valid()
{
	return rm->is_valid();
}

BINNACLE_API float binnacle_get_aspect_ratio()
{
	return rm->get_aspect_ratio();
}

BINNACLE_API int binnacle_create_shader_program(const std::string& vertex_shader_file, const std::string& fragment_shader_file,
								   const std::string& geometry_shader_file)
{
	return rm->create_shader_program(vertex_shader_file, fragment_shader_file, geometry_shader_file);
}

BINNACLE_API void binnacle_init_renderer()
{
	rm->init_renderer();
}

BINNACLE_API void binnacle_load_model_data(float *vertex_positions, const size_t vertex_count, unsigned int* indices, size_t const index_count,
								  const int mat_id)
{
	rm->load_model_data(vertex_positions, vertex_count, indices, index_count, mat_id);
}

BINNACLE_API unsigned char *binnacle_render_to_texture(const bool screen)
{
	//binnacle_update();
	binnacle_update();
	return rm->render_to_texture(screen);
}

BINNACLE_API void binnacle_dispose_texture(const unsigned char* pixels)
{
	delete[] pixels;
}

BINNACLE_API void binnacle_set_position(const float x, const float y, const float z)
{
	rm->get_camera().set_position(x, y, z);
}

BINNACLE_API void binnacle_set_focus(const float x, const float y, const float z)
{
	rm->get_camera().set_focus(x, y, z);
}

BINNACLE_API int binnacle_create_material(const unsigned program, const std::string& filename_texture, const std::string& filename_normals)
{
	return rm->create_material(program, filename_texture, filename_normals);
}

BINNACLE_API int binnacle_create_material_col(const unsigned program, const float r, const float g, const float b, const std::string& filename_normals)
{
	return rm->create_material(program, r, g, b, filename_normals);
}

BINNACLE_API void binnacle_delete_material(const int index)
{
	rm->delete_material(index);
}

BINNACLE_API void binnacle_update()
{
	rm->update();
}

BINNACLE_API void binnacle_render()
{
	rm->render();
}
