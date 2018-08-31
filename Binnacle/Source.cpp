// Manager
#include <iostream>

#include "RenderManager.hpp"

int main(int argc, char **argv)
{
	render_manager rm(false, 4, 3, 0, 640, 480, "OpenGL");
	
	rm.select_renderer(BINNACLE);
	
	rm.select_scene(std::make_shared<scene>());
	
	rm.set_environment(camera(glm::vec3(), glm::vec3(), glm::vec3(), 45, rm.get_aspect_ratio(), 1, 20));
	rm.set_lights(light(glm::vec3(0.0f, 0.1f, 4.0f), glm::vec3(1.0f, 1.0f, 1.0f), 40, 60),
				  light(glm::vec3(3.0f, 0.8f, -0.1f), glm::vec3(1.0f, 0.0f, .0f), 40, 70));
	
	const auto shader_program_id = rm.create_shader_program("vertex.glsl", "fragment.glsl");
	const auto mat_id_tex = rm.create_material(shader_program_id, "cube_tex.png");
	const auto mat_id_notex = rm.create_material(shader_program_id, glm::vec3(0.0f, 0.2f, 0.9f));
	
	rm.load_model("monkey.obj", true, mat_id_notex);
	//rm.load_model("cube.obj", false, mat_id_notex);
	//rm.load_model("sphere.obj", true, mat_id_notex);
	//rm.load_model("double_tri.obj", false, mat_id_notex);
	//rm.load_model("plane.obj", false, mat_id_notex);

	while (!rm.should_end())
	{
		rm.render();
		rm.update();
	}

	return 0;
}
