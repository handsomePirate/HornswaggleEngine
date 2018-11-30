#pragma once
#include <string>

typedef unsigned int rm_choice;

#define BINNACLE_API __declspec(dllexport)
#define GLM_ENABLE_EXPERIMENTAL

extern "C"
{
	typedef int * binnacle_ptr;
	typedef int * handle_ptr;

	BINNACLE_API binnacle_ptr binnacle_init(int width, int height, bool texture);
	BINNACLE_API void binnacle_release(binnacle_ptr binnacle);
	BINNACLE_API void binnacle_set_camera(binnacle_ptr binnacle, float px, float py, float pz, float fx, float fy, float fz, float ux, float uy, float uz);

	BINNACLE_API void binnacle_set_background_color(binnacle_ptr binnacle, float r, float g, float b);

	BINNACLE_API bool binnacle_is_valid(binnacle_ptr binnacle);

	BINNACLE_API float binnacle_get_aspect_ratio(binnacle_ptr binnacle);

	BINNACLE_API int binnacle_create_shader_program(binnacle_ptr binnacle, const std::string& vertex_shader_file, const std::string& fragment_shader_file, const std::string& geometry_shader_file = "");
	BINNACLE_API void binnacle_init_renderer(binnacle_ptr binnacle);

	BINNACLE_API int binnacle_load_model_data(binnacle_ptr binnacle, float *vertex_positions, size_t vertex_count, unsigned int *indices, size_t index_count, int mat_id = -1);
	BINNACLE_API int binnacle_load_model_vertices(binnacle_ptr binnacle, float *vertex_positions, float *normals, size_t vertex_count, unsigned int *indices, size_t index_count, int mat_id = -1);

	BINNACLE_API int binnacle_instance_model(binnacle_ptr binnacle, int model);

	BINNACLE_API handle_ptr binnacle_instance_handle(binnacle_ptr binnacle, int instance);
	BINNACLE_API void binnacle_assign_handle_position(handle_ptr handle, float x, float y, float z);
	BINNACLE_API void binnacle_rotate_handle(handle_ptr handle, float x, float y, float z, float angle);
	BINNACLE_API void binnacle_assign_handle_scale(handle_ptr handle, float x, float y, float z);
	BINNACLE_API void destroy_instance_handle(handle_ptr handle);

	BINNACLE_API unsigned char *binnacle_render_to_texture(binnacle_ptr binnacle, bool screen = false);
	BINNACLE_API void binnacle_dispose_texture(const unsigned char *pixels);

	BINNACLE_API void binnacle_render(binnacle_ptr binnacle);

	BINNACLE_API void binnacle_set_position(binnacle_ptr binnacle, float x, float y, float z);
	BINNACLE_API void binnacle_set_focus(binnacle_ptr binnacle, float x, float y, float z);

	BINNACLE_API int binnacle_create_material(binnacle_ptr binnacle, unsigned program, const std::string& filename_texture = "", const std::string& filename_normals = "");
	BINNACLE_API int binnacle_create_material_col(binnacle_ptr binnacle, unsigned program, float r, float g, float b, const std::string& filename_normals = "");
	BINNACLE_API void binnacle_delete_material(binnacle_ptr binnacle, int index);

	BINNACLE_API void binnacle_start_fps(binnacle_ptr binnacle);

	BINNACLE_API float binnacle_update(binnacle_ptr binnacle);

	BINNACLE_API bool binnacle_should_end(binnacle_ptr binnacle);
}