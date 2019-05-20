#include "DynamiteRenderer.hpp"
#include <iostream>

int main(int argc, char **argv)
{
	Dynamite::Renderer renderer("Hornswaggle engine", 1);
	renderer.SetTargetWindow(640, 480);
	renderer.Introduce();
	renderer.Render();

	return 0;
}