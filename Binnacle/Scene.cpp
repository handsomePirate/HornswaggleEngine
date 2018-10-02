#include <fstream>

#include "Scene.hpp"
#include "MathHelper.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

void split_from(const std::string& str, size_t index, const char c, std::string& before, std::string& after)
{
	while (index < str.size() && str[index] != c)
	{
		before += str[index++];
	}
	if (index + 1 < str.size())
		after = str.substr(index + 1, str.size() - index - 1);
}

camera::camera()
	: position_(), fov_(), aspect_(), z_near_(), z_far_() {}

camera::camera(glm::vec3&& position, glm::vec3&& focus, glm::vec3&& up, const float fov, const float aspect, const float z_near, const float z_far)
	: position_(position, 1), focus_(focus), up_(normalize(up)), fov_(PI * (fov / 180.0f)), aspect_(aspect), z_near_(z_near), z_far_(z_far)
{
	create_projection_matrix();
	create_view_matrix();
}

void camera::rotate(const glm::vec3& axis, const float angle)
{
	if (angle == 0)
		return;

	const auto rotation = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), angle, axis);

	focus_ = rotation * (glm::vec4(focus_, 1.0f) - position_) + position_;
	up_ = rotation * glm::vec4(up_, 1.0f);
}

void camera::rotate(const float x, const float y, const float z, const float angle)
{
	rotate(glm::vec3(x, y, z), angle);
}

void camera::rotate_local(const glm::vec3& axis, const float angle)
{
	const glm::vec3 new_axis = get_local_to_global_matrix() * glm::vec4(axis, 1.0f);

	rotate(new_axis, angle);
}

void camera::rotate_local(const float x, const float y, const float z, const float angle)
{
	rotate_local(glm::vec3(x, y, z), angle);
}

void camera::translate(const glm::vec3& offset)
{
	focus_.x += offset.x;
	focus_.y += offset.y;
	focus_.z += offset.z;

	position_.x += offset.x;
	position_.y += offset.y;
	position_.z += offset.z;
}

void camera::translate(const float dx, const float dy, const float dz)
{
	focus_.x += dx;
	focus_.y += dy;
	focus_.z += dz;

	position_.x += dx;
	position_.y += dy;
	position_.z += dz;
}

void camera::translate_local(const glm::vec3& offset)
{
	const glm::vec3 new_offset = get_local_to_global_matrix() * glm::vec4(offset, 1.0f);

	translate(new_offset);
}

void camera::translate_local(const float dx, const float dy, const float dz)
{
	translate_local(glm::vec3(dx, dy, dz));
}

void camera::translate_local_2_d(const glm::vec3& offset)
{
	glm::vec3 new_offset = get_local_to_global_matrix() * glm::vec4(offset, 1.0f);
	new_offset.y = 0.0f;
	new_offset = normalize(new_offset) * length(offset);

	translate(new_offset);
}

void camera::translate_local_2_d(const float dx, const float dy, const float dz)
{
	translate_local_2_d(glm::vec3(dx, dy, dz));
}

glm::mat4 camera::get_local_to_global_matrix() const
{
	const auto forward = normalize(focus_ - glm::vec3(position_));
	const auto aside = cross(up_, forward);

	const auto a11 = aside.x; const auto a12 = aside.y; const auto a13 = aside.z;
	const auto a21 = up_.x; const auto a22 = up_.y; const auto a23 = up_.z;
	const auto a31 = forward.x; const auto a32 = forward.y; const auto a33 = forward.z;
	const auto a41 = position_.x; const auto a42 = position_.y; const auto a43 = position_.z;

	return inverse(glm::mat4( // transposed
		a11, a21, a31, a41,
		a12, a22, a32, a42,
		a13, a23, a33, a43,
		  0,   0,   0,  1
	));
}

void camera::set_focus(glm::vec3& f)
{
	focus_ = f;
}

void camera::set_focus(const float x, const float y, const float z)
{
	set_focus(glm::vec3(x, y, z));
}

void camera::set_forward(glm::vec3& f)
{
	focus_ = glm::vec3(position_) + normalize(f);
}

void camera::set_position(glm::vec3& p)
{
	position_ = glm::vec4(p, 1);
}

void camera::set_position(const float x, const float y, const float z)
{
	position_ = glm::vec4(x, y, z, 1);
}

void camera::set_up(glm::vec3& u)
{
	up_ = u;
}

void camera::set_position_forward(glm::vec3& p, glm::vec3& f)
{
	position_ = glm::vec4(p, 1);
	focus_ = p + normalize(f);
}

void camera::set_position_focus(glm::vec3& p, glm::vec3& f)
{
	position_ = glm::vec4(p, 1);;
	focus_ = f;
}

void camera::set_position_up(glm::vec3& p, glm::vec3& u)
{
	position_ = glm::vec4(p, 1);
	up_ = u;
}

void camera::set_forward_up(glm::vec3& f, glm::vec3& u)
{
	focus_ = glm::vec3(position_) + normalize(f);
	up_ = u;
}

void camera::set_focus_up(glm::vec3& f, glm::vec3& u)
{
	focus_ = f;
	up_ = u;
}

void camera::set_transform_forward(glm::vec3& p, glm::vec3& f, glm::vec3& u)
{
	position_ = glm::vec4(p, 1);
	focus_ = p + normalize(f);
	up_ = u;
}

void camera::set_transform_focus(glm::vec3& p, glm::vec3& f, glm::vec3& u)
{
	position_ = glm::vec4(p, 1);
	focus_ = f;
	up_ = u;
}

void camera::set_focus(glm::vec3&& f)
{
	set_focus(f);
}

void camera::set_forward(glm::vec3&& f)
{
	set_forward(f);
}

void camera::set_position(glm::vec3&& p)
{
	set_position(p);
}

void camera::set_up(glm::vec3&& u)
{
	set_up(u);
}

void camera::set_position_forward(glm::vec3&& p, glm::vec3&& f)
{
	set_position_forward(p, f);
}

void camera::set_position_focus(glm::vec3&& p, glm::vec3&& f)
{
	set_position_focus(p, f);
}

void camera::set_position_up(glm::vec3&& p, glm::vec3&& u)
{
	set_position_up(p, u);
}

void camera::set_forward_up(glm::vec3&& f, glm::vec3&& u)
{
	set_forward_up(f, u);
}

void camera::set_focus_up(glm::vec3&& f, glm::vec3&& u)
{
	set_focus_up(f, u);
}

void camera::set_transform_forward(glm::vec3&& p, glm::vec3&& f, glm::vec3&& u)
{
	set_transform_forward(p, f, u);
}

void camera::set_transform_focus(glm::vec3&& p, glm::vec3&& f, glm::vec3&& u)
{
	set_transform_focus(p, f, u);
}

void camera::set_aspect(const float aspect)
{
	aspect_ = aspect;
}

void camera::set_near_far(const float z_near, const float z_far)
{
	z_near_ = z_near;
	z_far_ = z_far;
}

void camera::set_near(const float z_near)
{
	z_near_ = z_near;
}

void camera::set_far(const float z_far)
{
	z_far_ = z_far;
}

void camera::set_fov(const float fov)
{
	fov_ = fov;
}

void camera::set_frustum(float fov, float aspect, float z_near, float z_far)
{
	fov_ = fov;
	aspect_ = aspect;
	z_near_ = z_near;
	z_far_ = z_far;
}

const glm::vec4& camera::get_position() const
{
	return position_;
}

float camera::get_aspect() const
{
	return aspect_;
}

float camera::get_near() const
{
	return z_near_;
}

float camera::get_far() const
{
	return z_far_;
}

float camera::get_fov() const
{
	return fov_;
}

const glm::mat4& camera::get_view_matrix() const
{
	return view_matrix_;
}

const glm::mat4& camera::get_projection_matrix() const
{
	return projection_matrix_;
}

void camera::create_matrices()
{
	create_view_matrix();
	create_projection_matrix();
}

void camera::create_projection_matrix()
{
	projection_matrix_ = glm::perspective(fov_, aspect_, z_near_, z_far_);
}

void camera::create_view_matrix()
{
	view_matrix_ = lookAt(glm::vec3(position_), focus_, up_);
}

light::light(const glm::vec3& position, const glm::vec3& color, const float diffuse_intensity, const float specular_intensity)
	: specular_intensity_(specular_intensity), diffuse_intensity_(diffuse_intensity), color_(color), position_(position) {}

const glm::vec3& light::get_position() const
{
	return position_;
}

const glm::vec3& light::get_color() const
{
	return color_;
}

float light::get_diffuse_intensity() const
{
	return diffuse_intensity_;
}

float light::get_specular_intensity() const
{
	return specular_intensity_;
}

environment::environment(camera& cam)
	: camera_(cam), environment_map_(0)
{
}

camera& environment::get_camera()
{
	return camera_;
}

const std::vector<glm::vec3>& environment::get_light_positions() const
{
	return light_positions_;
}

const std::vector<glm::vec3>& environment::get_light_colors() const
{
	return light_colors_;
}

const std::vector<float>& environment::get_light_diffuse_intensities() const
{
	return light_diffuse_intensities_;
}

const std::vector<float>& environment::get_light_specular_intensities() const
{
	return light_specular_intensities_;
}

void environment::set_environment_map(const GLuint id)
{
	environment_map_ = id;
	has_environment_map_ = true;
}

bool environment::has_env_map() const
{
	return has_environment_map_;
}

void environment::shader_load_env_map(const GLuint program) const
{
	glActiveTexture(GL_TEXTURE31);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environment_map_);

	const auto cubemap_loc = glGetUniformLocation(program, "cubemap");
	glUniform1i(cubemap_loc, 31);
}

bool environment::changed() const
{
	return changed_;
}

void environment::unset_changed()
{
	changed_ = false;
}

#define DEBUG_LIMIT_INDICES_ 3

model::model(const std::string& filename_model, const bool smooth, const int mat_id)
	: material_id_(mat_id)
{
	std::ifstream ifs(filename_model);

	std::string line;
	std::string sign;

	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> positions;
	std::vector<glm::vec2> uv_coords;

	std::map<unsigned int, std::map<unsigned int, std::pair<int, std::vector<unsigned int>>>> index_hash;

	while (ifs.good())
	{
		if (ifs.peek() == 'v')
		{
			ifs >> sign;
			if (sign == "v")
			{
				float x, y, z;
				ifs >> x; ifs >> y; ifs >> z;
				positions.emplace_back(x, y, z, 1);
			}
			if (sign == "vn")
			{
				float x, y, z;
				ifs >> x; ifs >> y; ifs >> z;
				normals.emplace_back(x, y, z);
			}
			if (sign == "vt")
			{
				float x, y;
				ifs >> x; ifs >> y;
				uv_coords.emplace_back(x, -y);
			}
			continue;
		}
		if (ifs.peek() == 'f')
		{
			ifs >> sign;
			int first_i, second_i, third_i;
			int first_n, second_n, third_n;
			int first_u, second_u, third_u;

			ifs >> first_i;
			while (ifs.peek() == '/')
			{
				ifs.get();
				if (ifs.peek() == '/')
				{
					ifs.get();
					ifs >> first_n;
				}
				else
				{
					ifs >> first_u;
					ifs.get();
					ifs >> first_n;
				}
			}

			ifs >> second_i;
			while (ifs.peek() == '/')
			{
				ifs.get();
				if (ifs.peek() == '/')
				{
					ifs.get();
					ifs >> second_n;
				}
				else
				{
					ifs >> second_u;
					ifs.get();
					ifs >> second_n;
				}
			}

			ifs >> third_i;
			third_n = -1;
			third_u = -1;
			while (ifs.peek() == '/')
			{
				ifs.get();
				if (ifs.peek() == '/')
				{
					ifs.get();
					ifs >> third_n;
				}
				else
				{
					ifs >> third_u;
					ifs.get();
					ifs >> third_n;
				}
			}

#ifdef DEBUG_LIMIT_INDICES
			if (vertices.size() < DEBUG_LIMIT_INDICES)
			{
#endif
				// TODO: indexed loading
				if (third_n == -1)
				{
					if (smooth)
					{
						process_vertex(vertices_, positions[first_i - 1], glm::vec3(), glm::vec2(), glm::vec3(), first_i - 1, indices_, index_hash);
						process_vertex(vertices_, positions[second_i - 1], glm::vec3(), glm::vec2(), glm::vec3(), second_i - 1, indices_, index_hash);
						process_vertex(vertices_, positions[third_i - 1], glm::vec3(), glm::vec2(), glm::vec3(), third_i - 1, indices_, index_hash);
					}
					else
					{
						emplace_vertex(vertices_, vertex(positions[first_i - 1]), indices_);
						emplace_vertex(vertices_, vertex(positions[second_i - 1]), indices_);
						emplace_vertex(vertices_, vertex(positions[third_i - 1]), indices_);
					}
				}
				else
				{
					if (third_u == -1)
					{
						if (smooth)
						{
							process_vertex(vertices_, positions[first_i - 1], normals[first_n - 1], glm::vec2(), glm::vec3(), first_i - 1, indices_, index_hash);
							process_vertex(vertices_, positions[second_i - 1], normals[second_n - 1], glm::vec2(), glm::vec3(), second_i - 1, indices_, index_hash);
							process_vertex(vertices_, positions[third_i - 1], normals[third_n - 1], glm::vec2(), glm::vec3(), third_i - 1, indices_, index_hash);
						}
						else
						{
							emplace_vertex(vertices_, vertex(positions[first_i - 1], normals[first_n - 1]), indices_);
							emplace_vertex(vertices_, vertex(positions[second_i - 1], normals[second_n - 1]), indices_);
							emplace_vertex(vertices_, vertex(positions[third_i - 1], normals[third_n - 1]), indices_);
						}
					}
					else
					{
						const auto tangent = compute_tangent(positions[second_i - 1] - positions[first_i - 1], positions[third_i - 1] - positions[first_i - 1],
															 uv_coords[second_u - 1] - uv_coords[first_u - 1], uv_coords[third_u - 1] - uv_coords[first_u - 1]); // TODO: might need to interpolate later
						if (smooth)
						{
							process_vertex(vertices_, positions[first_i - 1], normals[first_n - 1], uv_coords[first_u - 1], tangent, first_i - 1, indices_, index_hash);
							process_vertex(vertices_, positions[second_i - 1], normals[second_n - 1], uv_coords[second_u - 1], tangent, second_i - 1, indices_, index_hash);
							process_vertex(vertices_, positions[third_i - 1], normals[third_n - 1], uv_coords[third_u - 1], tangent, third_i - 1, indices_, index_hash);
						}
						else
						{
							emplace_vertex(vertices_, vertex(positions[first_i - 1], normals[first_n - 1], uv_coords[first_u - 1], tangent), indices_);
							emplace_vertex(vertices_, vertex(positions[second_i - 1], normals[second_n - 1], uv_coords[second_u - 1], tangent), indices_);
							emplace_vertex(vertices_, vertex(positions[third_i - 1], normals[third_n - 1], uv_coords[third_u - 1], tangent), indices_);
						}
					}
				}

#ifdef DEBUG_LIMIT_INDICES
			}
#endif
			continue;
		}

		std::getline(ifs, line);
		// TODO: improve
	}

	// =======TESTING=======
	//vertices_.resize(3);
	//vertices_[0] = glm::vec3(-0.5f, -0.5f, 0.0f);
	//vertices_[1] = glm::vec3(0.0f, 0.5f, 0.0f);
	//vertices_[2] = glm::vec3(0.5f, -0.5f, 0.0f);
	//
	//indices_.resize(3);
	//indices_[0] = 0;
	//indices_[1] = 1;
	//indices_[2] = 2;

#ifdef DEBUG_LIMIT_INDICES
	vertex_count_ = DEBUG_LIMIT_INDICES;
	index_count_ = DEBUG_LIMIT_INDICES;
#endif
}

model_instance::~model_instance()
{
	if (prev)
		prev->next = next;
	if (next)
		next->prev = prev;

	for (auto && handle : registered_handles_)
	{
		handle->delete_instance_reference();
	}
}

void model_instance::insert_after(model_instance *mi)
{
	next = mi->next;
	mi->next = this;
	prev = mi;
}

model_instance *model_instance::insert_before(model_instance *mi)
{
	prev = mi->prev;
	mi->prev = this;
	next = mi;

	return prev;
}

int model_instance::transform_vertices_to(vertex *pos, const int index, const bool change)
{
	if (transformed_ || change || index != last_vertex_transform_index_)
	{
		const auto translate_mat = glm::translate(glm::mat4(1.0f), glm::vec3(position_));
		const auto scale_mat = glm::scale(glm::mat4(1.0f), scale_);
		model_matrix_ = translate_mat * toMat4(orientation_) * scale_mat; //glm::mat4(1.0f)
		auto curr = m->get_vertex_data();

		for (unsigned int i = 0; i < m->get_vertex_count(); ++i)
		{
			*pos = *curr;

			pos->set_position((model_matrix_ * curr->get_position()));
			pos->set_normal(orientation_ * curr->get_normal());
			pos->set_tangent(orientation_ * curr->get_tangent());

			++pos;
			++curr;
		}
	}
	
	transformed_ = false;
	last_vertex_transform_index_ = index;
	return index + m->get_vertex_count();
}

int model_instance::transform_indices_to(unsigned int* pos, const int index, const bool change)
{
	if (change || index != last_index_transform_index_)
	{
		auto curr = m->get_index_data();

		for (unsigned int i = 0; i < m->get_index_count(); ++i)
		{
			*pos = *curr + start_index_;

			++pos;
			++curr;
		}
	}

	last_index_transform_index_ = index;
	return index + m->get_index_count();
}

void model_instance::rotate(const glm::vec3& axis, const float angle)
{
	if (angle == 0)
		return;

	transformed_ = true;
	orientation_ = glm::rotate(orientation_, angle, axis);
}

void model_instance::rotate(const float x, const float y, const float z, const float angle)
{
	rotate(glm::vec3(x, y, z), angle);
}

void model_instance::translate(const glm::vec4& offset)
{
	if (offset.x == 0 && offset.y == 0 && offset.z == 0)
		return;

	transformed_ = true;
	position_ += offset;
}

void model_instance::translate(const float dx, const float dy, const float dz)
{
	if (dx == 0 && dy == 0 && dz == 0)
		return;

	transformed_ = true;
	position_.x += dx;
	position_.y += dy;
	position_.z += dz;
}

void model_instance::scale(const glm::vec3& ratio)
{
	if (ratio.x == 1 && ratio.y == 1 && ratio.z == 1)
		return;

	transformed_ = true;
	scale_ = glm::vec3(ratio.x * scale_.x, ratio.y * scale_.y, ratio.z * scale_.z);
}

void model_instance::scale(const float ratio)
{
	if (ratio == 1)
		return;

	transformed_ = true;
	scale_ *= ratio;
}

void model_instance::assign_position(const glm::vec4& position)
{
	if (position.x == position_.x && position.y == position_.y && position.z == position_.z)
		return;

	transformed_ = true;
	position_ = position;
}

void model_instance::assign_position(const float x, const float y, const float z)
{
	if (x == position_.x && y == position_.y && z == position_.z)
		return;

	transformed_ = true;
	position_ = glm::vec4(x, y, z, 1);
}

void model_instance::assign_orientation(const glm::quat& orientation)
{
	if (orientation.x == orientation_.x && orientation.y == orientation_.y && orientation.z == orientation_.z && orientation.w == orientation_.w)
		return;

	transformed_ = true;
	orientation_ = orientation;
}

void model_instance::assign_orientation(const float x, const float y, const float z, const float w)
{
	if (x == orientation_.x && y == orientation_.y && z == orientation_.z && w == orientation_.w)
		return;

	transformed_ = true;
	orientation_ = glm::quat(w, x, y, z);
}

void model_instance::assign_scale(const glm::vec3& scale)
{
	if (scale.x == scale_.x && scale.y == scale_.y && scale.z == scale_.z)
		return;

	transformed_ = true;
	scale_ = scale;
}

void model_instance::assign_scale(const float x, const float y, const float z)
{
	if (x == scale_.x && y == scale_.y && z == scale_.z)
		return;
	
	transformed_ = true;
	scale_ = glm::vec3(x, y, z);
}

void model_instance::register_handle(instance_handle* mh)
{
	registered_handles_.push_back(mh);
}

void model_instance::change_start_index_by(const int amount)
{
	start_index_ += amount;
	if (prev)
		prev->change_start_index_by(amount);
}

unsigned model_instance::get_start_index() const
{
	return start_index_;
}

unsigned model::get_vertex_count() const
{
	return vertices_.size();
}

unsigned model::get_index_count() const
{
	return indices_.size();
}

unsigned model::get_poly_count() const
{
	return indices_.size() / 3;
}

int model::get_material_index() const
{
	return material_id_;
}

vertex* model::get_vertex_data()
{
	return &vertices_[0];
}

unsigned int* model::get_index_data()
{
	return &indices_[0];
}

void model_instance::rotate(const glm::vec3&& axis, const float angle)
{
	rotate(axis, angle);
}

unsigned int model::hash_3(const vertex& v)
{
	const auto pos = v.get_position();

	const auto p1 = 3;
	const auto p2 = 7;
	const auto p3 = 13;

	return static_cast<unsigned int>(floor(pos.x * p1 + pos.y * p2 + pos.z * p3)) % 100; // TODO
}

unsigned int model::emplace_vertex(std::vector<vertex>& vertices, const vertex& v, std::vector<unsigned int>& indices)
{
	vertices.push_back(v);
	indices.push_back(vertices.size() - 1);

	return vertices.size() - 1;
}

void model::process_vertex(std::vector<vertex>& vertices, const glm::vec4& pos, const glm::vec3& norm, const glm::vec2& uv, const glm::vec3& tangent, const unsigned int id, 
	std::vector<unsigned int>& indices, std::map<unsigned int, std::map<unsigned int, std::pair<int, std::vector<unsigned int>>>>& index_hash)
{
	auto v = vertex(pos, norm, uv, tangent);

	const auto hash = hash_3(v);
	const auto it1 = index_hash.find(hash);
	if (it1 == index_hash.end())
	{
		const auto index = emplace_vertex(vertices, v, indices);

		index_hash[hash] = std::map<unsigned int, std::pair<int, std::vector<unsigned int>>>();
		//index_hash[hash][id] = std::pair<int, std::vector<unsigned int>>();
		index_hash[hash][id].first = 1;
		index_hash[hash][id].second.push_back(index);
	}
	else
	{
		const auto it2 = index_hash[hash].find(id);
		if (it2 == index_hash[hash].end())
		{
			const auto index = emplace_vertex(vertices, v, indices);

			//index_hash[hash][id] = std::pair<int, std::vector<unsigned int>>();
			//index_hash[hash][id].second = std::vector<unsigned int>();
			index_hash[hash][id].first = 1;
			index_hash[hash][id].second.push_back(index);
		}
		else
		{
			const int match_index = find_match(vertices, index_hash[hash][id].second, v);

			if (match_index == -1)
			{
				interpolate_normals(vertices, index_hash[hash][id].second, index_hash[hash][id].first, v);
				++index_hash[hash][id].first;
				const auto index = emplace_vertex(vertices, v, indices); // TODO: interpolate the entire vector and change you normal too
				index_hash[hash][id].second.push_back(index);
			}
			else
			{
				interpolate_normals(vertices, index_hash[hash][id].second, index_hash[hash][id].first, v);
				++index_hash[hash][id].first;
				indices.push_back(index_hash[hash][id].second[match_index]); // TODO: add weight to normal
			}
		}
	}
}

int model::find_match(std::vector<vertex>& vertices, std::vector<unsigned int>& indices, const vertex& v)
{
	for (size_t i = 0; i < indices.size(); ++i)
	{
		if (compare_vertices(vertices[indices[i]], v))
			return i;
	}

	return -1;
}

void model::interpolate_normals(std::vector<vertex>& vertices, std::vector<unsigned int>& indices, const int weight,
	vertex& v)
{
	if (indices.empty())
		return;

	const auto old_norm = vertices[indices[0]].get_normal();
	const auto int_norm = v.get_normal();

	if (compare_vectors(old_norm, int_norm))
		return;

	const float old_x = old_norm.x;
	const float old_y = old_norm.y;
	const float old_z = old_norm.z;

	const float int_x = int_norm.x;
	const float int_y = int_norm.y;
	const float int_z = int_norm.z;

	const auto new_norm = normalize(glm::vec3((old_x * weight + int_x) / static_cast<float>(weight + 1), 
		(old_y * weight + int_y) / static_cast<float>(weight + 1), 
		(old_z * weight + int_z) / static_cast<float>(weight + 1)));

	v.set_normal(new_norm);

	for (auto && i : indices)
	{
		vertices[i].set_normal(new_norm);
	}
}

const float tolerance = 0.001f;
bool model::compare_vectors(const glm::vec2& v1, const glm::vec2& v2)
{
	return fabs(v1.x - v2.x) < tolerance && fabs(v1.y - v2.y) < tolerance;
}

bool model::compare_vectors(const glm::vec3& v1, const glm::vec3& v2)
{
	return fabs(v1.x - v2.x) < tolerance && fabs(v1.y - v2.y) < tolerance && fabs(v1.z - v2.z) < tolerance;
}

bool model::compare_vectors(const glm::vec4& v1, const glm::vec4& v2)
{
	return compare_vectors(glm::vec3(v1), glm::vec3(v2));
}

bool model::compare_vertices(const vertex& v1, const vertex& v2)
{
	return compare_vectors(v1.get_position(), v2.get_position()) &&
		compare_vectors(v1.get_color(), v2.get_color()) &&
		compare_vectors(v1.get_uv_coords(), v2.get_uv_coords());
}

glm::vec3 model::compute_tangent(const glm::vec3& edge1, const glm::vec3& edge2, const glm::vec2& delta_uv1, const glm::vec2& delta_uv2)
{
	glm::vec3 tangent;

	const float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

	tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
	tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
	tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);
	return normalize(tangent);
}

scene::scene()
{
	pck_ptr_ = std::make_unique<std::map<int, material_pack>>();
	ins_ptr_ = std::make_unique<std::map<int, model_instance *>>();
	use_material(-1);
}

int scene::instance_model(model* m)
{
	ASSIGN_FREE_ID(mod_free_ids_, mod_next_free_id_);

	const auto mi = new model_instance(m, (*pck_ptr_)[m->get_material_index()].vertex_count);
	(*pck_ptr_)[m->get_material_index()].vertex_count += m->get_vertex_count();
	(*pck_ptr_)[m->get_material_index()].index_count += m->get_index_count();

	if ((*pck_ptr_)[m->get_material_index()].model_instances_linked_list_start)
		mi->insert_before((*pck_ptr_)[m->get_material_index()].model_instances_linked_list_start);
	else
		(*pck_ptr_)[m->get_material_index()].model_instances_linked_list_end = mi;

	(*pck_ptr_)[m->get_material_index()].model_instances_linked_list_start = mi;

	(*ins_ptr_)[id] = mi;

	return id;
}

model_instance *scene::get_instance(const int index) const
{
	if (ins_ptr_->find(index) != ins_ptr_->end())
		return (*ins_ptr_)[index];
	return nullptr;
}

void scene::delete_model_instance(const int index)
{
	const auto it = ins_ptr_->find(index);
	if (it == ins_ptr_->end())
		return;

	const auto& m = it->second->m;

	const int vc = m->get_vertex_count();
	if (it->second->prev)
		it->second->prev->change_start_index_by(-vc);

	(*pck_ptr_)[m->get_material_index()].vertex_count -= m->get_vertex_count();
	(*pck_ptr_)[m->get_material_index()].index_count -= m->get_index_count();

	if (!it->second->prev)
		(*pck_ptr_)[m->get_material_index()].model_instances_linked_list_start = it->second->next;
	if (!it->second->next)
		(*pck_ptr_)[m->get_material_index()].model_instances_linked_list_end = it->second->prev;
	
	mod_free_ids_.push(index);

	delete (*ins_ptr_)[index];
	ins_ptr_->erase(it);
}

void scene::update() const
{
	if (pck_ptr_->size() > 1)
	{
		//if (!(*pck_ptr_)[0].models_.empty())
		//{
		//	(*pck_ptr_)[0].models_[0].rotate(1, 0, 0, 0.01f);
		//	(*pck_ptr_)[0].models_[0].assign_position(-0.6, 0, 0);
		//	(*pck_ptr_)[0].models_[0].assign_scale(0.4, 0.4, 0.4);
		//	if (pck_ptr_->size() > 2 && !(*pck_ptr_)[1].models_.empty())
		//	{
		//		(*pck_ptr_)[1].models_[0].assign_position(0.6, 0, 0);
		//		(*pck_ptr_)[1].models_[0].assign_scale(0.4, 0.4, 0.4);
		//	}
		//}
	}
}

void scene::use_material(const int mat_id) const
{
	(*pck_ptr_)[mat_id] = material_pack();
}

void scene::disable_material(const int mat_id) const
{
	pck_ptr_->erase(mat_id);
}

const std::vector<vertex>& scene::get_vertices(const int mat_id) const
{
	auto change = false;
	if ((*pck_ptr_)[mat_id].transformed_vertices.size() != (*pck_ptr_)[mat_id].vertex_count)
	{
		(*pck_ptr_)[mat_id].transformed_vertices.resize((*pck_ptr_)[mat_id].vertex_count);
		change = true;
	}
#ifdef DEBUG_LIMIT_INDICES
	transformed_vertices_.resize(DEBUG_LIMIT_INDICES);
#endif
	int index = 0;
	auto mi = (*pck_ptr_)[mat_id].model_instances_linked_list_end;

	while (mi != nullptr)
	{
		index = mi->transform_vertices_to(&(*pck_ptr_)[mat_id].transformed_vertices[index], index, change);
		mi = mi->prev;
	}

	return (*pck_ptr_)[mat_id].transformed_vertices;
}

const std::vector<unsigned int>& scene::get_indices(const int mat_id) const
{
	auto change = false;
	if ((*pck_ptr_)[mat_id].indices.size() != (*pck_ptr_)[mat_id].index_count)
	{
		(*pck_ptr_)[mat_id].indices.resize((*pck_ptr_)[mat_id].index_count);
		change = true;
	}

	int index = 0;
	auto mi = (*pck_ptr_)[mat_id].model_instances_linked_list_end;

	while (mi != nullptr)
	{
		index = mi->transform_indices_to(&(*pck_ptr_)[mat_id].indices[index], index, change);
		mi = mi->prev;
	}

	return (*pck_ptr_)[mat_id].indices;
}
