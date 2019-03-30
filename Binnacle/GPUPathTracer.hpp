#pragma once
#include "Renderer.hpp"

struct gpu_path_tracer : renderer
{
	explicit gpu_path_tracer(GLuint texture_draw_program, GLuint path_tracer_program, int width, int height);
	gpu_path_tracer(const gpu_path_tracer& rm) = delete;
	gpu_path_tracer& operator=(const gpu_path_tracer& rm) = delete;
	gpu_path_tracer(gpu_path_tracer && rm) = delete;
	gpu_path_tracer& operator=(gpu_path_tracer && rm) = delete;
	~gpu_path_tracer();

	void render(const std::shared_ptr<scene>& scn_ptr,
		const std::shared_ptr<std::map<int, material>>& mat_ptr,
		const std::shared_ptr<environment>& env_ptr) override;

	void enable(const vizualization& option) const override;
	void disable(const vizualization& option) const override;

	bool is_enabled(const vizualization& option) const override;

	void set_background_color(const glm::vec3& color) override;

	void change_viewport_size(unsigned int width, unsigned int height) override;

	unsigned long long get_time_elapsed() const override;
private:
	// TODO: destroy these objects
	GLuint tex_;
	GLuint path_tracer_program_;
	int width_;
	int height_;

	bool *vizualization_options_;
	glm::vec3 background_color_{};

	GLuint time_query_;
	GLuint64 time_elapsed_ = 0;
};
