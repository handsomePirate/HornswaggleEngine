// Manager
#include "Binnacle_Render_Manager.hpp"

#include "RenderManager.hpp"
#include <SOIL/SOIL.h>
#include "MathHelper.hpp"
#include <windows.h>
#include <thread>
#include <glm/gtx/norm.hpp>

#define KKAPP
#undef KKAPP

#ifndef KKAPP
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

	rm.set_lights(light(glm::vec3(1.0f, 0.1f, 9.0f), glm::vec3(1.0f, 1.0f, 1.0f), 40, 60),
		light(glm::vec3(-3.0f, 0.8f, 15), glm::vec3(0.75f, 0, 1.0f), 40, 50));
	rm.set_camera(glm::vec3(), glm::vec3(), glm::vec3(), 45, rm.get_aspect_ratio(), 0.1, 100);
	
	const auto shader_program_id = rm.create_shader_program("vertex.glsl", "fragment_brdf.glsl");
	const auto mat_id_tex = rm.create_material(shader_program_id, "leather_a.png", "leather_nr.png");
	//const auto mat_id_notex = rm.create_material(shader_program_id, glm::vec3(0.06f, 0.3f, 0.9f));
	const auto mat_id_notex = rm.create_material(shader_program_id, glm::vec3(1.0f, 0.0f, 0.0f));

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
		//if (total_time > 0.1f && counter > 0)
		//{
		//	total_time = 0;
		//	rm.delete_model_instance(counter--);
		//}

		const auto angle = 40.0f;
		//model1_instance_handle.rotate(1, 0, 0, angle / 360.0f * PI * time_elapsed);
		for (auto && handle : handles)
		{
			//handle.rotate(0, 1, 0, angle / 360.0f * PI * time_elapsed);
			//handle.rotate(1, 0, 0, angle / 360.0f * PI * time_elapsed);
		}
		rm.render();
		// Stabilize the framerate somewhat
		if (time_elapsed < FRAMERATE_STEP)
			Sleep((FRAMERATE_STEP - time_elapsed) * 1000);
	}

	return 0;
}
#else
#include <fstream>
#include <iostream>

glm::vec3 parse_line(std::ifstream& ifs, int& err);
void create_cylinder(std::vector<vertex>& vertices, std::vector<unsigned int>& indices, float radius, int detail);

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		std::cout << "No files selected!" << std::endl;
		std::cin.get();
		return 0;
	}

	std::vector<std::vector<glm::vec3>> position_sets;
	std::vector<glm::vec3> positions;
	std::vector<float> maxes;

	float radius = 0.06f;
	int detail = 8;
	float axisWidth = 0.01f;
	float minX = -5;
	float maxX = 5;
	float maxY = 2;
	float minZ = -5;
	float maxZ = 5;

	std::ifstream ifs("config.txt");

	while (ifs.good())
	{
		std::string cmd;
		char c;
		int counter = 0;
		while ((c = ifs.get()) != ':')
		{
			cmd += c;
			++counter;
		}
		std::string val;
		std::getline(ifs, val);

		if (cmd == "radius")
		{
			radius = stof(val);
		}
		if (cmd == "detail")
		{
			detail = stoi(val);
		}
		if (cmd == "axisWidth")
		{
			axisWidth = stof(val);
		}
		if (cmd == "minX")
		{
			minX = stof(val);
		}
		if (cmd == "maxX")
		{
			maxX = stof(val);
		}
		if (cmd == "minZ")
		{
			minZ = stof(val);
		}
		if (cmd == "maxZ")
		{
			maxZ = stof(val);
		}
		if (cmd == "maxY")
		{
			maxY = stof(val);
		}
	}


	const int meshCount = argc - 1;
	float min_x = FLT_MAX;
	float min_y = FLT_MAX;
	float min_z = FLT_MAX;

	float max_x = -(FLT_MAX / 10);
	float max_y = -(FLT_MAX / 10);
	float max_z = -(FLT_MAX / 10);

	for (int i = 1; i < argc; i++)
	{
		std::ifstream ifs_arg(argv[i]);

		int counter = 0;
		while (ifs_arg.good())
		{
			int err;
			const glm::vec3 v = parse_line(ifs_arg, err);
			if (err == 0)
			{
				if (v.x < min_x)
					min_x = v.x;
				if (v.y < min_y)
					min_y = v.y;
				if (v.z < min_z)
					min_z = v.z;

				if (v.x > max_x)
					max_x = v.x;
				if (v.y > max_y)
					max_y = v.y;
				if (v.z > max_z)
					max_z = v.z;

				positions.push_back(v);
			}
			else
			{
				if (err == 1)
					std::cout << "Line to short! Not enough information!" << std::endl;

				if (err == 2)
					std::cout << "Parsing error! Could not recoginze a floating point number!" << std::endl;
			}
		}

		position_sets.emplace_back(positions);

		positions.clear();
	}
	if (abs(min_y) > max_y)
		max_y = abs(min_y);
	if (min_x == max_x)
		max_x += 0.0001;
	if (min_z == max_z)
		max_z += 0.0001;

	std::vector<vertex> vertices;
	std::vector<unsigned int> indices;
	create_cylinder(vertices, indices, radius, detail);
 	render_manager rm(false, 4, 3, 0, 640, 480, "OpenGL");

	rm.init_renderer();

	rm.set_camera(glm::vec3(), glm::vec3(), glm::vec3(), 45, rm.get_aspect_ratio(), 0.1, 100);
	const auto shader_program_id = rm.create_shader_program("vertex.glsl", "fragment_fresnel_color.glsl");

	int vertexCount = 0;
	glm::vec3 colors[] = { glm::vec3(0, 1, 0), glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec3(1, 1, 0), glm::vec3(1, 0, 1), glm::vec3(0, 1, 1) };

	for (int i = 0; i < meshCount; i++)
	{
		auto m = rm.load_model_data(vertices, indices, rm.create_material(shader_program_id, colors[i]));
		for (int j = 0; j < position_sets[i].size(); ++j)
		{
			const auto instance = rm.instance_model(m);
			auto handle = rm.get_instance_handle(instance);

			handle.assign_position((position_sets[i][j].x - min_x) * ((maxX - minX) / (max_x - min_x)) + minX, 0, (position_sets[i][j].z - min_z) * ((maxZ - minZ) / (max_z - min_z)) + minZ);

			if (position_sets[i][j].y < 0)
			{
				position_sets[i][j].y = -position_sets[i][j].y;
				handle.rotate(1, 0, 0, PI);
			}
			handle.assign_scale(1, position_sets[i][j].y * (maxY / max_y), 1);
		}
	}
	const auto m = rm.load_model_data(vertices, indices, rm.create_material(shader_program_id, glm::vec3(0, 0, 0)));
	
	const auto instance1 = rm.instance_model(m);
	auto handle1 = rm.get_instance_handle(instance1);
	handle1.rotate(0, 0, 1, -PI / 2.0f);
	handle1.assign_scale(axisWidth, maxX - minX, axisWidth);
	handle1.assign_position(minX, 0, minZ);
	
	const auto instance2 = rm.instance_model(m);
	auto handle2 = rm.get_instance_handle(instance2);
	handle2.rotate(1, 0, 0, PI / 2.0f);
	handle2.assign_scale(axisWidth, maxZ - minZ, axisWidth);
	handle2.assign_position(minX, 0, minZ);

	rm.start_framerate();
	while (!rm.should_end())
	{
		const auto time_elapsed = rm.update();

		rm.render();
		// Stabilize the framerate somewhat
		if (time_elapsed < FRAMERATE_STEP)
			Sleep((FRAMERATE_STEP - time_elapsed) * 1000);
	}
}

glm::vec3 parse_line(std::ifstream& ifs, int& err)
{
	err = 0;

	float x;
	ifs >> x;

	float y;
	ifs >> y;

	float z;
	ifs >> z;

	return glm::vec3(x, y, z);
}

void create_cylinder(std::vector<vertex>& vertices, std::vector<unsigned int>& indices, const float radius, const int detail)
{
	for (int i = 0; i < detail; i++)
	{
		const float rad = 2 * PI * i / detail;

		const float x = cos(rad) * radius;
		const float z = sin(rad) * radius;

		const float y = sqrt(pow(x, 2) + pow(z, 2));

		vertices.emplace_back(glm::vec4(x, 0, z, 1), normalize(glm::vec3(x, -y, z)));
		vertices.emplace_back(glm::vec4(x, 1, z, 1), normalize(glm::vec3(x, y, z)));
	}

	vertices.emplace_back(glm::vec4(0, 0, 0, 1), glm::vec3(0, -1, 0));
	vertices.emplace_back(glm::vec4(0, 1, 0, 1), glm::vec3(0, 1, 0));

	for (int i = 0; i < detail; i++)
	{
		const int i1 = 2 * i;
		int i2 = 2 * i + 2;
		const int i3 = 2 * i + 1;
		int i4 = 2 * i + 3;

		if (i == detail - 1)
		{
			i2 = 0;
			i4 = 1;
		}

		indices.push_back(i1);
		indices.push_back(i2);
		indices.push_back(vertices.size() - 2);

		indices.push_back(i4);
		indices.push_back(i3);
		indices.push_back(vertices.size() - 1);
		
		indices.push_back(i2);
		indices.push_back(i1);
		indices.push_back(i3);
		
		indices.push_back(i2);
		indices.push_back(i3);
		indices.push_back(i4);
	}
}
#endif