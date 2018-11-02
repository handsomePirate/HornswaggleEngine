// Manager
#include "Binnacle_Render_Manager.hpp"

#include "RenderManager.hpp"
#include <SOIL/SOIL.h>
#include "MathHelper.hpp"

int main(int argc, char **argv)
{
	//binnacle_init(640, 480, true);
	//
	//const auto data = new float[24]{
	//	-1, 0, 1,
	//	1, 0, 1,
	//
	//	-1, 0, -1,
	//	1, 0, -1 };
	//
	//binnacle_clear_scene();
	//
	//const auto grid_cols = 2;
	//const auto grid_rows = 2;
	//
	//const auto index_count = (grid_cols - 1) * (grid_rows - 1) * 6;
	//const auto indices = new unsigned int[index_count];
	//for (auto i = 0; i < (grid_cols - 1) * (grid_rows - 1); ++i)
	//{
	//	const auto i1 = i + i / grid_cols;
	//	const auto i2 = i1 + 1;
	//	const auto i3 = i + grid_cols + i / grid_cols;
	//	const auto i4 = i3 + 1;
	//
	//	indices[i * 6] = i1;
	//	indices[i * 6 + 1] = i2;
	//	indices[i * 6 + 2] = i3;
	//	indices[i * 6 + 3] = i4;
	//	indices[i * 6 + 4] = i3;
	//	indices[i * 6 + 5] = i2;
	//}
	//const auto vertex_count = 2 * 2;
	//
	//binnacle_load_model_data(data, vertex_count, indices, index_count, 0);
	//delete[] indices;
	//delete[] data;
	//
	//binnacle_set_position(0, 0.1, 5);
	//binnacle_set_focus(0, 0, 0);
	//
	//binnacle_update();
	//const auto pixels = binnacle_render_to_texture();
	//SOIL_save_image("screen.bmp", SOIL_SAVE_TYPE_BMP, 640, 480, 4, pixels);
	//
	//binnacle_release();
	//
	//return 0;

	render_manager rm(false, 4, 3, 0, 640, 480, "OpenGL");

	rm.init_renderer();

	rm.set_lights(light(glm::vec3(1.0f, 0.1f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), 40, 60),
		light(glm::vec3(3.0f, 0.8f, -0.1f), glm::vec3(0.75f, 0.75f, 1.0f), 40, 50));
	rm.set_camera(glm::vec3(), glm::vec3(), glm::vec3(), 45, rm.get_aspect_ratio(), 0.1, 100);
	
	const auto shader_program_id = rm.create_shader_program("vertex.glsl", "fragment_brdf.glsl");
	const auto mat_id_tex = rm.create_material(shader_program_id, "leather_a.png", "leather_nr.png");
	const auto mat_id_notex = rm.create_material(shader_program_id, glm::vec3(0.06f, 0.3f, 0.9f));

	const auto model = rm.load_model("Human_body.obj", true, mat_id_notex);

	const int instance_count = 4;
	std::vector<instance_handle> handles;
	const float circle_size = 1.6f;
	for (int i = 0; i < instance_count; ++i)
	{
		const auto instance = rm.instance_model(model);
		handles.push_back(rm.get_instance_handle(instance));
		const auto angle = i / static_cast<float>(instance_count) * 2 * PI;
		handles[handles.size() - 1].assign_position(sin(angle) * circle_size, 0, -cos(angle) * circle_size + 5);
		handles[handles.size() - 1].scale(0.4);
	}

	//rm.load_model("monkey.obj", true, mat_id_tex);
	//const auto model1 = rm.load_model("even_cube.obj", false, mat_id_tex);
	//const auto model2 = rm.load_model("cube.obj", true, mat_id_notex);
	//
	//const auto model1_instance = rm.instance_model(model1);
	//const auto model2_instance = rm.instance_model(model2);
	//
	//const auto model1_instance_handle = rm.get_instance_handle(model1_instance);
	//const auto model2_instance_handle = rm.get_instance_handle(model2_instance);
	//
	//model1_instance_handle.assign_position(-0.6, 0, 0);
	//model1_instance_handle.assign_scale(0.4, 0.4, 0.4);
	//
	//model2_instance_handle.assign_position(0.6, 0, 0);
	//model2_instance_handle.assign_scale(0.4, 0.4, 0.4);

	//rm.load_model("cube.obj", true, mat_id_notex);
	//rm.load_model("double_tri.obj", false, mat_id_notex);
	//rm.load_model("plane.obj", false, mat_id_notex);

	//rm.load_environment_cube_map("cubemap//neg_z.png", "cubemap//pos_z.png", "cubemap//neg_x.png", "cubemap//pos_x.png", "cubemap//neg_y.png", "cubemap//pos_y.png");
	//rm.load_environment_cube_map("cubemap//uni.png", "cubemap//uni.png", "cubemap//uni.png", "cubemap//uni.png", "cubemap//uni.png", "cubemap//uni.png");
	//rm.load_environment_cube_map("cubemap//z.png", "cubemap//z.png", "cubemap//x.png", "cubemap//x.png", "cubemap//y.png", "cubemap//y.png");
	rm.load_environment_cube_map(
		"cubemap//cubemap_neg_z.png", "cubemap//cubemap_pos_z.png", 
		"cubemap//cubemap_neg_x.png", "cubemap//cubemap_pos_x.png", 
		"cubemap//cubemap_neg_y.png", "cubemap//cubemap_pos_y.png");

	//rm.init_texture_rendering(640, 480);
	//delete rm.render_to_texture(true);

	auto counter = instance_count - 1;
	auto total_time = 0.0f;

	rm.start_framerate();
	while (!rm.should_end())
	{
		const auto time_elapsed = rm.update();
		total_time += time_elapsed;
		if (total_time > 0.1f && counter > 0)
		{
			total_time = 0;
			rm.delete_model_instance(counter--);
		}

		const auto angle = 40.0f;
		//model1_instance_handle.rotate(1, 0, 0, angle / 360.0f * PI * time_elapsed);
		for (auto && handle : handles)
		{
			//handle.rotate(0, 1, 0, angle / 360.0f * PI * time_elapsed);
			//handle.rotate(1, 0, 0, angle / 360.0f * PI * time_elapsed);
		}
		rm.render();
	}

	return 0;
}
