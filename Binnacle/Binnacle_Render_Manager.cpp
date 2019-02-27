#ifdef LIB
#include "Binnacle_Render_Manager.hpp"
#include "RenderManager.hpp"

BINNACLE_API binnacle_ptr binnacle_init(const int width, const int height, std::string& root_path, const bool texture)
{
	render_manager *binnacle = new render_manager(false, 4, 3, 0, width, height, "OpenGL", root_path, !texture);
	return reinterpret_cast<binnacle_ptr>(binnacle);
}

BINNACLE_API void binnacle_release(binnacle_ptr binnacle)
{
	delete reinterpret_cast<render_manager *>(binnacle);
}

BINNACLE_API void binnacle_set_camera(binnacle_ptr binnacle, 
	const float px, const float py, const float pz, 
	const float fx, const float fy, const float fz, 
	const float ux, const float uy, const float uz)
{
	reinterpret_cast<render_manager *>(binnacle)->set_camera(glm::vec3(px, py, pz), glm::vec3(fx, fy, fz), glm::vec3(ux, uy, uz), 
		45, reinterpret_cast<render_manager *>(binnacle)->get_aspect_ratio(), 0.1, 100);
}

BINNACLE_API void binnacle_set_background_color(binnacle_ptr binnacle, float r, float g, float b)
{
	reinterpret_cast<render_manager *>(binnacle)->set_background_color(glm::vec3(r, g, b));
}

BINNACLE_API bool binnacle_is_valid(binnacle_ptr binnacle)
{
	return reinterpret_cast<render_manager *>(binnacle)->is_valid();
}

BINNACLE_API float binnacle_get_aspect_ratio(binnacle_ptr binnacle)
{
	return reinterpret_cast<render_manager *>(binnacle)->get_aspect_ratio();
}

BINNACLE_API void binnacle_register_path(binnacle_ptr binnacle, std::string& name, std::string& path_from_root)
{
	reinterpret_cast<render_manager *>(binnacle)->register_path(name, path_from_root);
}

BINNACLE_API int binnacle_create_shader_program(binnacle_ptr binnacle, const std::string& vertex_shader_file, const std::string& fragment_shader_file,
								   const std::string& geometry_shader_file)
{
	return reinterpret_cast<render_manager *>(binnacle)->create_shader_program(vertex_shader_file, fragment_shader_file, geometry_shader_file);
}

BINNACLE_API void binnacle_init_renderer(binnacle_ptr binnacle)
{
	reinterpret_cast<render_manager *>(binnacle)->init_opengl_renderer();
}

BINNACLE_API int binnacle_load_model_data(binnacle_ptr binnacle, float *vertex_positions, const size_t vertex_count, unsigned int* indices, size_t const index_count,
								  const int mat_id)
{
	return reinterpret_cast<render_manager *>(binnacle)->load_model_data(vertex_positions, vertex_count, indices, index_count, mat_id);
}

BINNACLE_API int binnacle_load_model_vertices(binnacle_ptr binnacle, float *vertex_positions, float *normals, size_t vertex_count, unsigned int *indices, 
									size_t index_count, int mat_id)
{
	return reinterpret_cast<render_manager *>(binnacle)->load_model_data(vertex_positions, normals, vertex_count, indices, index_count, mat_id);
}

BINNACLE_API int binnacle_instance_model(binnacle_ptr binnacle, int model)
{
	return reinterpret_cast<render_manager *>(binnacle)->instance_model(model);
}

BINNACLE_API handle_ptr binnacle_instance_handle(binnacle_ptr binnacle, int instance)
{
	auto handle = reinterpret_cast<render_manager *>(binnacle)->get_instance_handle(instance);
	instance_handle *result = (instance_handle *)malloc(sizeof(instance_handle)); // WARNING: This is unsafe.. if the instance is deleted, the program will crash
	std::swap(*result, handle);

	return  (handle_ptr)result;
}

BINNACLE_API void binnacle_assign_handle_position(handle_ptr handle, float x, float y, float z)
{
	reinterpret_cast<instance_handle *>(handle)->assign_position(x, y, z);
}

BINNACLE_API void binnacle_rotate_handle(handle_ptr handle, float x, float y, float z, float angle)
{
	reinterpret_cast<instance_handle *>(handle)->rotate(x, y, z, angle);
}

BINNACLE_API void binnacle_assign_handle_scale(handle_ptr handle, float x, float y, float z)
{
	reinterpret_cast<instance_handle *>(handle)->assign_scale(x, y, z);
}

BINNACLE_API void destroy_instance_handle(handle_ptr handle)
{
	free(handle);
}

BINNACLE_API unsigned char *binnacle_render_to_texture(binnacle_ptr binnacle, const bool screen)
{
	return reinterpret_cast<render_manager *>(binnacle)->render_to_texture(screen);
}

BINNACLE_API void binnacle_dispose_texture(const unsigned char* pixels)
{
	delete[] pixels;
}

BINNACLE_API void binnacle_set_position(binnacle_ptr binnacle, const float x, const float y, const float z)
{
	reinterpret_cast<render_manager *>(binnacle)->get_camera().set_position(x, y, z);
}

BINNACLE_API void binnacle_set_focus(binnacle_ptr binnacle, const float x, const float y, const float z)
{
	reinterpret_cast<render_manager *>(binnacle)->get_camera().set_focus(x, y, z);
}

BINNACLE_API int binnacle_create_material(binnacle_ptr binnacle, const unsigned program, const std::string& filename_texture, const std::string& filename_normals)
{
	return reinterpret_cast<render_manager *>(binnacle)->create_material(program, filename_texture, filename_normals);
}

BINNACLE_API int binnacle_create_material_col(binnacle_ptr binnacle, const unsigned program, const float r, const float g, const float b, const std::string& filename_normals)
{
	return reinterpret_cast<render_manager *>(binnacle)->create_material(program, r, g, b, filename_normals);
}

BINNACLE_API void binnacle_delete_material(binnacle_ptr binnacle, const int index)
{
	reinterpret_cast<render_manager *>(binnacle)->delete_material(index);
}

BINNACLE_API void binnacle_start_fps(binnacle_ptr binnacle)
{
	reinterpret_cast<render_manager *>(binnacle)->start_framerate();
}

BINNACLE_API float binnacle_update(binnacle_ptr binnacle)
{
	return reinterpret_cast<render_manager *>(binnacle)->update();
}

BINNACLE_API void binnacle_render(binnacle_ptr binnacle)
{
	reinterpret_cast<render_manager *>(binnacle)->render();
}

BINNACLE_API bool binnacle_should_end(binnacle_ptr binnacle)
{
	return reinterpret_cast<render_manager *>(binnacle)->should_end();
}
#endif