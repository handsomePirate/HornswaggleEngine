// Manager
#include "RenderManager.hpp"

bool end = false;
bool move = false;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
		end = true;
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		move = !move;
}

int main(int argc, char **argv)
{
	render_manager rm(false, 4, 3, 0, 640, 480, "OpenGL", key_callback);
	rm.select_renderer(BINNACLE);
	rm.select_scene(std::make_shared<scene>());
	rm.set_environment(camera(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 45, rm.get_aspect_ratio(), 1, 20), glm::vec3(1, 0, 0));
	rm.load_model("cube.obj", "cube_tex.png");

	const float movement_granularity = 0.02f;
	float moved = 0;

	while (!end)
	{
		rm.get_camera().set_position(glm::vec3(sin(moved) * 5, 0, cos(moved) * 5));
		if (move)
			moved += movement_granularity;

		rm.render();
		rm.update();
	}

	return 0;
}