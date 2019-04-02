#include "GPUPathTracer.hpp"
#include <chrono>

gpu_path_tracer::gpu_path_tracer(const GLuint texture_draw_program, const GLuint path_tracer_program, const int width, const int height)
	: renderer(texture_draw_program), path_tracer_program_(path_tracer_program), width_(width), height_(height)
{
	// Prepare the rendering parameters and texture
	vizualization_options_ = new bool[2]
	{
		/*LIGHTS = */false,
		/*ENVIRONMENT_MAP = */false
	};

	background_color_ = glm::vec3(0.1f, 0.1f, 0.1f);

	glGenTextures(1, &tex_);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height * samples, 0, GL_RGBA, GL_FLOAT, nullptr);

#ifdef MEASURE_SHADER_TIME
	// Prepare time measurement device
	glGenQueries(1, &time_query_);
#endif
}

gpu_path_tracer::~gpu_path_tracer()
{
	delete[] vizualization_options_;
}

void gpu_path_tracer::render(const std::shared_ptr<scene>& scn_ptr,
	const std::shared_ptr<std::map<int, material>>& mat_ptr,
	const std::shared_ptr<environment>& env_ptr)
{
#ifdef MEASURE_SHADER_TIME
	// Start shader timer
	glBeginQuery(GL_TIME_ELAPSED, time_query_);
#endif
	// Setup program
	glBindImageTexture(0, tex_, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glUseProgram(path_tracer_program_);

	// Update shader camera
	glm::vec4 eye = env_ptr->get_camera().get_position();
	const auto eye_loc = glGetUniformLocation(path_tracer_program_, "eye");
	glUniform3fv(eye_loc, 1, &eye[0]);

	const glm::mat4 inverse_view_projection_matrix = env_ptr->get_camera().get_view_projection_inverse_matrix();

	glm::vec4 ray00 = inverse_view_projection_matrix * glm::vec4(-1, -1, 0, 1);
	ray00 /= ray00.w;
	ray00 -= eye;
	const auto ray00_loc = glGetUniformLocation(path_tracer_program_, "ray00");
	glUniform3fv(ray00_loc, 1, &ray00[0]);

	glm::vec4 ray01 = inverse_view_projection_matrix * glm::vec4(-1, 1, 0, 1);
	ray01 /= ray01.w;
	ray01 -= eye;
	const auto ray01_loc = glGetUniformLocation(path_tracer_program_, "ray01");
	glUniform3fv(ray01_loc, 1, &ray01[0]);
	
	glm::vec4 ray10 = inverse_view_projection_matrix * glm::vec4(1, -1, 0, 1);
	ray10 /= ray10.w;
	ray10 -= eye;
	const auto ray10_loc = glGetUniformLocation(path_tracer_program_, "ray10");
	glUniform3fv(ray10_loc, 1, &ray10[0]);
	
	glm::vec4 ray11 = inverse_view_projection_matrix * glm::vec4(1, 1, 0, 1);
	ray11 /= ray11.w;
	ray11 -= eye;
	const auto ray11_loc = glGetUniformLocation(path_tracer_program_, "ray11");
	glUniform3fv(ray11_loc, 1, &ray11[0]);

	const auto back_color_loc = glGetUniformLocation(path_tracer_program_, "backColor");
	glUniform3fv(back_color_loc, 1, &background_color_[0]);

	// Update the shader clock for the light movement
	const auto now = std::chrono::system_clock::now();
	const auto now_milli = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	const auto epoch = now_milli.time_since_epoch();
	const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
	const auto time_loc = glGetUniformLocation(path_tracer_program_, "time");
	glUniform1i(time_loc, time);

	auto samples_loc = glGetUniformLocation(path_tracer_program_, "samples");
	glUniform1i(samples_loc, samples);

	auto bounces_loc = glGetUniformLocation(path_tracer_program_, "bounces_uni");
	glUniform1i(bounces_loc, bounces);

	// Compute
	glDispatchCompute((GLuint)width_ / 16, (GLuint)height_ / 16, samples);

	// make sure writing to image has finished before read
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	render_texture(tex_, samples);

#ifdef MEASURE_SHADER_TIME
	// Query time elapsed for rendering
	glEndQuery(GL_TIME_ELAPSED);

	GLint done = 0;
	while (!done)
		glGetQueryObjectiv(time_query_, GL_QUERY_RESULT_AVAILABLE, &done);

	glGetQueryObjectui64v(time_query_, GL_QUERY_RESULT, &time_elapsed_);
#endif
}

void gpu_path_tracer::enable(const vizualization& option) const
{
	vizualization_options_[static_cast<int>(option)] = true;
}

void gpu_path_tracer::disable(const vizualization& option) const
{
	vizualization_options_[static_cast<int>(option)] = false;
}

bool gpu_path_tracer::is_enabled(const vizualization& option) const
{
	return vizualization_options_[static_cast<int>(option)];
}

void gpu_path_tracer::set_background_color(const glm::vec3& color)
{
	background_color_ = color;
}

void gpu_path_tracer::change_viewport_size(const unsigned int width, const unsigned int height)
{
	// Change the size of the texture
	width_ = width;
	height_ = height;
	glDeleteTextures(1, &tex_);
	glGenTextures(1, &tex_);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height * samples, 0, GL_RGBA, GL_FLOAT, nullptr);
}

unsigned long long gpu_path_tracer::get_time_elapsed() const
{
	// Return in milliseconds
	return time_elapsed_ / 1000000;
}
