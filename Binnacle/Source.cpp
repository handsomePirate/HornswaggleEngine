#include "Binnacle_Render_Manager.hpp"

#include "RenderManager.hpp"
#include "MathHelper.hpp"
#include "PathManipulator.hpp"
#include <Windows.h>
#include <thread>
#include <glm/gtx/norm.hpp>
#include <iostream>
#include "OpenGLRenderer.hpp"

#ifndef LIB
int main(int argc, char **argv)
{
	std::string binnacle_root = argv[0];
	path_manipulator::cut_last(binnacle_root);
	render_manager rm(false, 4, 3, 0, 640, 480, "OpenGL", binnacle_root);
	rm.register_path("shaders", "..\\..\\shaders");
	rm.register_path("models", "..\\..\\models");
	rm.register_path("hdr_textures", "..\\..\\textures\\hdr");
	rm.register_path("ldr_textures", "..\\..\\textures\\ldr");

	//rm.init_opengl_renderer();
	rm.init_path_tracer();

	//rm.init_path_tracing("path_trace_compute.glsl");
	//rm.path_trace();
	//
	//return 0;

	rm.set_lights(light(glm::vec3(1.0f, 0.1f, 9.0f), glm::vec3(1.0f, 1.0f, 1.0f), 20),
		light(glm::vec3(-3.0f, 0.8f, 15.0f), glm::vec3(0.75f, 0, 1.0f), 40));
	rm.set_camera(glm::vec3(), glm::vec3(), glm::vec3(), 45, rm.get_aspect_ratio(), 0.1, 100);

	rm.renderer_enable(vizualization::LIGHTS);
	
	const auto shader_program_id = rm.create_shader_program("vertex.glsl", "fragment_lambert_cook_torrance_GGX.glsl");
	const auto mat_id_notex = rm.create_material(shader_program_id, glm::vec3(1.0f, 1.0f, 1.0f)); // glm::vec3(1.0f, 0.843f, 0.0f)

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
	
	rm.load_hdr_environment("noon_grass_2k.hdr", 2048, 1024, "diffuse.bmp");

	auto counter = instance_count - 1;
	auto total_time = 0.0f;

	rm.start_framerate();
	while (!rm.should_end())
	{
		const auto time_elapsed = rm.update();
		total_time += time_elapsed;

		const auto angle = 40.0f;
		rm.render();
		// Stabilize the framerate somewhat
		if (time_elapsed < FRAMERATE_STEP)
			Sleep((FRAMERATE_STEP - time_elapsed) * 1000);
	}

	return 0;
}
#endif