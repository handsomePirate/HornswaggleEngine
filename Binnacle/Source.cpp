// Manager
#include <iostream>

#include "RenderManager.hpp"

int main(int argc, char **argv)
{
	render_manager rm(false, 4, 3, 0, 640, 480, "OpenGL");
	
	rm.select_renderer(BINNACLE);
	
	rm.select_scene(std::make_shared<scene>()); // TODO: reduce
	
	rm.set_environment(camera(glm::vec3(), glm::vec3(), glm::vec3(), 45, rm.get_aspect_ratio(), 1, 20));  // TODO: reduce
	rm.set_lights(light(glm::vec3(1.0f, 0.1f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), 40, 60),
				  light(glm::vec3(3.0f, 0.8f, -0.1f), glm::vec3(1.0f, 0.0f, .0f), 40, 50));
	
	const auto shader_program_id = rm.create_shader_program("vertex.glsl", "fragment_brdf.glsl");
	const auto mat_id_tex = rm.create_material(shader_program_id, "cube_tex.png");
	const auto mat_id_notex = rm.create_material(shader_program_id, glm::vec3(0.0f, 0.2f, 0.9f));
	
	//rm.load_model("monkey.obj", true, mat_id_tex);
	rm.load_model("cube.obj", false, mat_id_tex);
	rm.load_model("sharp_bevel_cube.obj", true, mat_id_notex);
	//rm.load_model("double_tri.obj", false, mat_id_notex);
	//rm.load_model("plane.obj", false, mat_id_notex);

	rm.load_environment_cube_map("cubemap//neg_z.png", "cubemap//pos_z.png", "cubemap//neg_x.png", "cubemap//pos_x.png", "cubemap//neg_y.png", "cubemap//pos_y.png");
	//rm.load_environment_cube_map("cubemap//uni.png", "cubemap//uni.png", "cubemap//uni.png", "cubemap//uni.png", "cubemap//uni.png", "cubemap//uni.png");
	//rm.load_environment_cube_map("cubemap//z.png", "cubemap//z.png", "cubemap//x.png", "cubemap//x.png", "cubemap//y.png", "cubemap//y.png");
	//rm.load_environment_cube_map("cubemap//cubemap_neg_z.png", "cubemap//cubemap_pos_z.png", "cubemap//cubemap_neg_x.png", "cubemap//cubemap_pos_x.png", "cubemap//cubemap_neg_y.png", "cubemap//cubemap_pos_y.png");

	while (!rm.should_end())
	{
		rm.render();
		rm.update();
	}

	return 0;
}
