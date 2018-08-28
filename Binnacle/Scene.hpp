#pragma once
#include <string>
#include <vector>

#include <glm/glm.hpp>

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

	const glm::vec3& get_position() const;

	float get_aspect() const;
	float get_near() const;
	float get_far() const;
	float get_fov() const;

	const glm::mat4& get_view_matrix() const;
	const glm::mat4& get_projection_matrix() const;
private:
	glm::vec3 position_;
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

struct scene
{
	explicit scene(const std::string& filename); // TODO: move scene loading to another project
	scene(const scene& rm) = delete;
	scene& operator=(const scene& rm) = delete;
	scene(scene && rm) = default;
	scene& operator=(scene && rm) = default;
	virtual ~scene() = default;

	const std::vector<glm::vec3>& get_vertices() const;
	const std::vector<unsigned short>& get_indices() const;
private:
	std::vector<glm::vec3> vertices_;
	std::vector<unsigned short> indices_;
	glm::vec3 camera_{};
};
