// Manager
#include "RenderManager.hpp"

bool end = false;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
		end = true;
}

int main(int argc, char **argv)
{
	render_manager rm(false, 4, 3, 0, 640, 480, "OpenGL", key_callback);
	rm.select_renderer(BINNACLE);
	rm.select_scene(std::make_shared<scene>("object.obj"));

	while (!end)
	{
		rm.render();
		rm.update();
	}

	return 0;
}