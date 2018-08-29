#pragma once
#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Texture.hpp"

struct camera
{
	camera(glm::vec3&& position, glm::vec3&& focus, glm::vec3&& up, float fov, float aspect, float z_near, float z_far);

	void set_focus(glm::vec3& f);
	void set_forward(glm::vec3& f);
	void set_position(glm::vec3& p);
	void set_up(glm::vec3& u);
	void set_position_forward(glm::vec3& p, glm::vec3& f);
	void set_position_focus(glm::vec3& p, glm::vec3& f);
	void set_position_up(glm::vec3& p, glm::vec3& u);
	void set_forward_up(glm::vec3& f, glm::vec3& u);
	void set_focus_up(glm::vec3& f, glm::vec3& u);
	void set_transform_forward(glm::vec3& p, glm::vec3& f, glm::vec3& u);
	void set_transform_focus(glm::vec3& p, glm::vec3& f, glm::vec3& u);

	void set_focus(glm::vec3&& f);
	void set_forward(glm::vec3&& f);
	void set_position(glm::vec3&& p);
	void set_up(glm::vec3&& u);
	void set_position_forward(glm::vec3&& p, glm::vec3&& f);
	void set_position_focus(glm::vec3&& p, glm::vec3&& f);
	void set_position_up(glm::vec3&& p, glm::vec3&& u);
	void set_forward_up(glm::vec3&& f, glm::vec3&& u);
	void set_focus_up(glm::vec3&& f, glm::vec3&& u);
	void set_transform_forward(glm::vec3&& p, glm::vec3&& f, glm::vec3&& u);
	void set_transform_focus(glm::vec3&& p, glm::vec3&& f, glm::vec3&& u);

	void set_aspect(float aspect);
	void set_near_far(float z_near, float z_far);
	void set_near(float z_near);
	void set_far(float z_far);
	void set_fov(float fov);

	void set_frustum(float fov, float aspect, float z_near, float z_far);

	const glm::vec4& get_position() const;

	float get_aspect() const;
	float get_near() const;
	float get_far() const;
	float get_fov() const;

	const glm::mat4& get_view_matrix() const;
	const glm::mat4& get_projection_matrix() const;
private:
	glm::vec4 position_;
	//glm::vec3 forward_;
	glm::vec3 focus_;
	glm::vec3 up_;
	float fov_;
	float aspect_;
	float z_near_;
	float z_far_;

	glm::mat4 view_matrix_{};
	glm::mat4 projection_matrix_{};

	void create_projection_matrix();
	void create_view_matrix();
};

struct environment
{
	explicit environment(camera& cam);
	camera& get_camera();

	template <class ... T>
	void add_lights(glm::vec3& first, T&&... args);
	static void add_lights() {}

	template <class ... T>
	void set_lights(T&&... args);

	const std::vector<glm::vec3>& get_lights() const;
private:
	std::vector<glm::vec3> lights_;
	camera camera_;
};

template <class ... T>
void environment::add_lights(glm::vec3& first, T&&... args)
{
	lights_.push_back(first);
	add_lights(std::forward<T>(args)...);
}

template <class ... T>
void environment::set_lights(T&&... args)
{
	if (!lights_.empty())
		lights_.clear();
	add_lights(std::forward<T>(args)...);
}

struct vertex
{
	vertex()
		: position_(0, 0, 0, 0), normal_(0, 0, 0), uv_coords_(0, 0) {}

	explicit vertex(const glm::vec4 position)
		: position_(position) {}

	vertex(const float x, const float y, const float z)
		: position_(glm::vec4(x, y, z, 1)) {}

	vertex(const glm::vec4 position, const glm::vec3 normal, const glm::vec3 color, const glm::vec2 uv_coords)
		: position_(position), normal_(normal), color_(color), uv_coords_(uv_coords) {}

	vertex(const glm::vec4 position, const glm::vec3 normal)
		: position_(position), normal_(normal) {}

	vertex(const glm::vec4 position, const glm::vec3 normal, const glm::vec2 uv_coords)
		: position_(position), normal_(normal), uv_coords_(uv_coords) {}

	const glm::vec4& get_position() const
	{
		return position_;
	}
	const glm::vec3& get_normal() const
	{
		return normal_;
	}
	const glm::vec3& get_color() const
	{
		return color_;
	}
	const glm::vec2& get_uv_coords() const
	{
		return uv_coords_;
	}

	void set_position(const glm::vec4 position)
	{
		position_ = position;
	}
	void set_position(const float x, const float y, const float z)
	{
		position_ = glm::vec4(x, y, z, 1);
	}
	void set_normal(const glm::vec3 normal)
	{
		normal_ = normal;
	}
	void set_normal(const float x, const float y, const float z)
	{
		normal_ = glm::vec3(x, y, z);
	}
	void set_color(const glm::vec3 color)
	{
		color_ = color;
	}
	void set_color(const float r, const float g, const float b)
	{
		color_ = glm::vec3(r, g, b);
	}
	void set_uv_coords(const glm::vec2 uv_coords)
	{
		uv_coords_ = uv_coords;
	}
	void set_uv_coords(const float x, const float y)
	{
		uv_coords_ = glm::vec2(x, y);
	}

private:
	glm::vec4 position_;
	glm::vec3 normal_{};
	glm::vec3 color_{};
	glm::vec2 uv_coords_{};
};

struct model
{
	model(std::vector<vertex>& vertices, std::vector<unsigned short>& indices, const std::string& filename_model, const std::string& filename_texture = ""); // TODO: move scene loading to another project
	int transform_vertices_to(vertex *pos, int index);

	void rotate(const glm::vec3& axis, float angle);
	void rotate(float x, float y, float z, float angle);
	void translate(const glm::vec4& offset);
	void translate(float dx, float dy, float dz);
	void scale(const glm::vec3& ratio);
	void scale(float ratio);

private:
	void rotate(const glm::vec3&& axis, float angle);

	vertex *vertices_;
	unsigned short *indices_;
	std::unique_ptr<texture> tex_ptr_;

	unsigned int vertex_count_;
	unsigned int index_count_;

	glm::mat4 model_matrix_{};
	glm::quat orientation_;
	glm::vec4 position_;
	glm::vec3 scale_;
};

struct scene
{
	scene() = default;
	scene(const scene& rm) = delete;
	scene& operator=(const scene& rm) = delete;
	scene(scene && rm) = default;
	scene& operator=(scene && rm) = default;
	virtual ~scene() = default;

	void load_model(const std::string& filename_model, const std::string& filename_texture = "");
	void update();

	const std::vector<vertex>& get_vertices();
	const std::vector<unsigned short>& get_indices() const;
private:
	std::vector<model> models_;
	std::vector<vertex> vertices_;
	std::vector<vertex> transformed_vertices_;
	std::vector<unsigned short> indices_;
};
