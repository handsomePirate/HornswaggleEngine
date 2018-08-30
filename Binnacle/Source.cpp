// Manager
#include "RenderManager.hpp"

int main(int argc, char **argv)
{
	render_manager rm(false, 4, 3, 0, 640, 480, "OpenGL");
	
	rm.select_renderer(BINNACLE);
	
	rm.select_scene(std::make_shared<scene>());
	
	rm.set_environment(camera(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 45, rm.get_aspect_ratio(), 1, 20));
	rm.set_lights(glm::vec3(0.0, 0.1, 4), glm::vec3(3.0, 1.0, -0.1));
	
	const auto shader_program_id = rm.create_shader_program("vertex.glsl", "fragment.glsl");
	const auto mat_id_tex = rm.create_material(shader_program_id, "cube_tex.png");
	const auto mat_id_notex = rm.create_material(shader_program_id, glm::vec3(0.0f, 0.2f, 0.9f));
	
	rm.load_model("cube.obj", mat_id_tex);
	rm.load_model("cube.obj", mat_id_notex);

	while (!rm.should_end())
	{
		rm.render();
		rm.update();
	}

	return 0;
}