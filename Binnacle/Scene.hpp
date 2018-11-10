#pragma once
#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Texture.hpp"
#include "RenderHelper.hpp"
#include <map>
#include <functional>
#include <queue>

#define ASSIGN_FREE_ID(free_vec, next_free)\
	int id = -1;\
	if (free_vec.empty())\
		{id = next_free++;}\
	else {id = free_vec.front(); free_vec.pop();}

// The fictional camera representation
struct camera
{
	camera();
	camera(glm::vec3&& position, glm::vec3&& focus, glm::vec3&& up, float fov, float aspect, float z_near, float z_far);

	void rotate(const glm::vec3& axis, float angle);
	void rotate(float x, float y, float z, float angle);

	void rotate_local(const glm::vec3& axis, float angle);
	void rotate_local(float x, float y, float z, float angle);

	void translate(const glm::vec3& offset);
	void translate(float dx, float dy, float dz);

	void translate_local(const glm::vec3& offset);
	void translate_local(float dx, float dy, float dz);

	void translate_local_2_d(const glm::vec3& offset);
	void translate_local_2_d(float dx, float dy, float dz);

	glm::mat4 get_local_to_global_matrix() const;

	void set_focus(glm::vec3& f);
	void set_focus(float x, float y, float z);
	void set_forward(glm::vec3& f);
	void set_position(glm::vec3& p);
	void set_position(float x, float y, float z);
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

	void create_matrices();
private:
	glm::vec4 position_;
	//glm::vec3 forward_;
	glm::vec3 focus_{};
	glm::vec3 up_{};
	float fov_;
	float aspect_;
	float z_near_;
	float z_far_;

	glm::mat4 view_matrix_{};
	glm::mat4 projection_matrix_{};

	void create_projection_matrix();
	void create_view_matrix();
};

struct light
{
	light(const glm::vec3& position, const glm::vec3& color, float diffuse_intensity, float specular_intensity);

	const glm::vec3& get_position() const;
	const glm::vec3& get_color() const;
	float get_diffuse_intensity() const;
	float get_specular_intensity() const;
private:
	float specular_intensity_;
	float diffuse_intensity_;
	glm::vec3 color_;

	glm::vec3 position_;
};

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

	vertex(const glm::vec4 position, const glm::vec3 normal, const glm::vec2 uv_coords, const glm::vec3 tangent)
		: position_(position), normal_(normal), uv_coords_(uv_coords), tangent_(tangent) {}

	const glm::vec4& get_position() const
	{
		return position_;
	}
	void translate(const float dx, const float dy, const float dz)
	{
		position_.x += dx;
		position_.y += dy;
		position_.z += dz;
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
	const glm::vec3& get_tangent() const
	{
		return tangent_;
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
	void set_tangent(const float x, const float y, const float z)
	{
		tangent_ = glm::vec3(x, y, z);
	}
	void set_tangent(const glm::vec3 tangent)
	{
		tangent_ = tangent;
	}

private:
	glm::vec4 position_;
	glm::vec3 normal_{};
	glm::vec3 color_{};
	glm::vec2 uv_coords_{};

	glm::vec3 tangent_{};
};

struct model
{
	model() = default;
	model(const std::string& filename_model, bool smooth, int mat_id); // TODO: move scene loading to another project
	// TODO: optimize loading (only this model needs to be considered, not other models)
	model(const std::vector<vertex>& vertices, const std::vector<unsigned int>& indices);

	unsigned int get_vertex_count() const;
	unsigned int get_index_count() const;
	unsigned int get_poly_count() const;
	int get_material_index() const;

	vertex *get_vertex_data();
	unsigned int *get_index_data();
private:
	static unsigned int hash_3(const vertex& v);

	static unsigned int emplace_vertex(std::vector<vertex>& vertices, const vertex& v, std::vector<unsigned int>& indices);

	static void process_vertex(std::vector<vertex>& vertices, const glm::vec4& pos, const glm::vec3& norm, const glm::vec2& uv, const glm::vec3& tangent, 
		unsigned int id, std::vector<unsigned int>& indices, std::map<unsigned int, std::map<unsigned int, std::pair<int, std::vector<unsigned int>>>>& index_hash);

	static int find_match(std::vector<vertex>& vertices, std::vector<unsigned int>& indices, const vertex& v);

	static void interpolate_normals(std::vector<vertex>& vertices, std::vector<unsigned int>& indices, int weight, vertex& v);

	static bool compare_vectors(const glm::vec2& v1, const glm::vec2& v2);
	static bool compare_vectors(const glm::vec3& v1, const glm::vec3& v2);
	static bool compare_vectors(const glm::vec4& v1, const glm::vec4& v2);
	static bool compare_vertices(const vertex& v1, const vertex& v2);

	static glm::vec3 compute_tangent(const glm::vec3& edge1, const glm::vec3& edge2, const glm::vec2& delta_uv1, const glm::vec2& delta_uv2);
	static glm::vec3 compute_tangent(const glm::vec3& normal);

	std::vector<vertex> vertices_;
	std::vector<unsigned int> indices_;

	int material_id_{};
};

struct environment
{
	explicit environment(camera& cam, GLuint program);
	camera& get_camera();

	template <class ... T>
	void add_lights(light& first, T&&... args);
	static void add_lights() {}

	template <class ... T>
	void set_lights(T&&... args);

	const std::vector<glm::vec3>& get_light_positions() const;
	const std::vector<glm::vec3>& get_light_colors() const;
	const std::vector<float>& get_light_diffuse_intensities() const;
	const std::vector<float>& get_light_specular_intensities() const;

	void set_environment_map(GLuint id);
	bool has_env_map() const;

	void shader_load_env_map(GLuint program) const;
	bool changed() const;
	void unset_changed();

	model& get_environment_cube();
	GLuint get_shader_program() const;
private:
	std::vector<glm::vec3> light_positions_;
	std::vector<glm::vec3> light_colors_;
	std::vector<float> light_diffuse_intensities_;
	std::vector<float> light_specular_intensities_;
	camera camera_;

	bool has_environment_map_ = false;
	GLuint environment_map_;

	model cube_;
	GLuint cube_program_;

	bool changed_ = true;
};

template <class ... T>
void environment::add_lights(light& first, T&&... args)
{
	light_positions_.push_back(first.get_position());
	light_colors_.push_back(first.get_color());
	light_diffuse_intensities_.push_back(first.get_diffuse_intensity());
	light_specular_intensities_.push_back(first.get_specular_intensity());

	add_lights(std::forward<T>(args)...);
}

template <class ... T>
void environment::set_lights(T&&... args)
{
	if (!light_positions_.empty())
		light_positions_.clear();
	if (!light_colors_.empty())
		light_colors_.clear();
	if (!light_diffuse_intensities_.empty())
		light_diffuse_intensities_.clear();
	if (!light_specular_intensities_.empty())
		light_specular_intensities_.clear();

	add_lights(std::forward<T>(args)...);
}

struct model_instance
{
	model_instance()
		: model_instance(nullptr, 0) {}

	model_instance(model *m, const unsigned int start_index)
		: prev(nullptr), next(nullptr), m(m), orientation_(1, 0, 0, 0), position_(0), scale_(1), start_index_(start_index) {}

	~model_instance();

	void insert_after(model_instance * mi);
	model_instance *insert_before(model_instance * mi);

	int transform_vertices_to(vertex *pos, int index, bool change);
	int transform_indices_to(unsigned int *pos, int index, bool change);

	void rotate(const glm::vec3& axis, float angle);
	void rotate(float x, float y, float z, float angle);
	void translate(const glm::vec4& offset);
	void translate(float dx, float dy, float dz);
	void scale(const glm::vec3& ratio);
	void scale(float ratio);

	void assign_position(const glm::vec4& position);
	void assign_position(float x, float y, float z);
	void assign_orientation(const glm::quat& orientation);
	void assign_orientation(float x, float y, float z, float w);
	void assign_scale(const glm::vec3& scale);
	void assign_scale(float x, float y, float z);

	void register_handle(instance_handle *mh);

	void change_start_index_by(int amount);

	unsigned int get_start_index() const;

	model_instance *prev;
	model_instance *next;

	model *m;

private:
	void rotate(const glm::vec3&& axis, float angle);

	glm::mat4 model_matrix_{};
	glm::quat orientation_{};
	glm::vec4 position_{};
	glm::vec3 scale_{};

	bool transformed_ = true;
	int last_vertex_transform_index_ = -1;
	int last_index_transform_index_ = -1;

	std::vector<instance_handle *> registered_handles_;

	unsigned int start_index_ = 0; // TODO: change with deletion
};

struct material_pack
{
	model_instance *model_instances_linked_list_start;
	model_instance *model_instances_linked_list_end;
	unsigned int vertex_count;
	unsigned int index_count;

	std::vector<vertex> transformed_vertices;
	std::vector<unsigned int> indices;
};

struct scene
{
	scene();
	scene(const scene& rm) = delete;
	scene& operator=(const scene& rm) = delete;
	scene(scene && rm) = default;
	scene& operator=(scene && rm) = default;
	virtual ~scene() = default;

	int instance_model(model *m);
	model_instance *get_instance(int index) const;
	void delete_model_instance(int index);

	void update() const;

	void use_material(int mat_id) const;
	void disable_material(int mat_id) const;

	const std::vector<vertex>& get_vertices(int mat_id) const;
	const std::vector<unsigned int>& get_indices(int mat_id) const;
private:
	std::unique_ptr<std::map<int, material_pack>> pck_ptr_;

	std::unique_ptr<std::map<int, model_instance *>> ins_ptr_;
	std::queue<int> mod_free_ids_;
	int mod_next_free_id_ = 0;
};
