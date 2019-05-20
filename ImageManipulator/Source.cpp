#include "Tests.hpp"
#include <chrono>

int main(int argc, char **argv)
{
	const auto start = std::chrono::high_resolution_clock::now();
	//auto test = sizeof(ImageFileFormats::dib_header_40);
	//load_test("test255.bmp");
	//load_test("LAND3.BMP");
	const unsigned int resolution = 255;
	save_test("test" + std::to_string(resolution) + ".bmp", resolution);
	//for (unsigned int i = 1; i <= 50; ++i)
	//{
	//	save_test("test" + std::to_string(i) + ".bmp", i);
	//}
	const auto end = std::chrono::high_resolution_clock::now();

	std::cout << "Tests took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms." << std::endl;

	return 0;
}
