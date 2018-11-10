#pragma once
#include <string>

typedef unsigned int rm_choice;
#define BINNACLE (unsigned int)1

#define BINNACLE_API __declspec(dllexport)
#define GLM_ENABLE_EXPERIMENTAL

extern "C"
{
	BINNACLE_API int *binnacle_init(int width, int height, bool texture);
	BINNACLE_API void binnacle_release(int *binnacle);

	BINNACLE_API bool binnacle_is_valid(int *binnacle);

	BINNACLE_API float binnacle_get_aspect_ratio(int *binnacle);

	BINNACLE_API int binnacle_create_shader_program(int *binnacle, const std::string& vertex_shader_file, const std::string& fragment_shader_file, const std::string& geometry_shader_file = "");
	BINNACLE_API void binnacle_init_renderer(int *binnacle);

	BINNACLE_API void binnacle_load_model_data(int *binnacle, float *vertex_positions, size_t vertex_count, unsigned int *indices, size_t index_count, int mat_id = -1);

	BINNACLE_API unsigned char *binnacle_render_to_texture(int *binnacle, bool screen = false);
	BINNACLE_API void binnacle_dispose_texture(const unsigned char *pixels);

	BINNACLE_API void binnacle_render(int *binnacle);

	BINNACLE_API void binnacle_set_position(int *binnacle, float x, float y, float z);
	BINNACLE_API void binnacle_set_focus(int *binnacle, float x, float y, float z);

	BINNACLE_API int binnacle_create_material(int *binnacle, unsigned program, const std::string& filename_texture = "", const std::string& filename_normals = "");
	BINNACLE_API int binnacle_create_material_col(int *binnacle, unsigned program, float r, float g, float b, const std::string& filename_normals = "");
	BINNACLE_API void binnacle_delete_material(int *binnacle, int index);

	//static void binnacle_load_environment_cube_map(const std::string& neg_z, const std::string& pos_z,
	//	const std::string& neg_x, const std::string& pos_x,
	//	const std::string& neg_y, const std::string& pos_y);

	//static float binnacle_get_fps();

	BINNACLE_API void binnacle_update(int *binnacle);
	//static void binnacle_render();

	//bool binnacle_should_end();
}