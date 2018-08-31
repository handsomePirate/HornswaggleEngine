#include <fstream>

#include "Scene.hpp"
#include "MathHelper.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

void split_from(const std::string& str, size_t index, const char c, std::string& before, std::string& after)
{
	while (index < str.size() && str[index] != c)
	{
		before += str[index++];
	}
	if (index + 1 < str.size())
		after = str.substr(index + 1, str.size() - index - 1);
}

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

void camera::set_forward(glm::vec3& f)
{
	focus_ = glm::vec3(position_) + normalize(f);
}

void camera::set_position(glm::vec3& p)
{
	position_ = glm::vec4(p, 1);
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
	view_matrix_ = lookAt(glm::vec3(position_), glm::vec3(focus_), glm::vec3(up_));
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
	: camera_(cam) { }

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

#define DEBUG_LIMIT_INDICES_ 3

model::model(std::vector<vertex>& vertices, std::vector<unsigned short>& indices, const std::string& filename_model, bool smooth, const int mat_id)
{
	std::ifstream ifs(filename_model);

	std::string line;
	std::string sign;

	vertices_index_ = vertices.size();
	indices_index_ = indices.size();

	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> positions;
	std::vector<glm::vec2> uv_coords;

	std::map<unsigned int, std::map<unsigned short, std::pair<int, std::vector<unsigned short>>>> index_hash;

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
						process_vertex(vertices, positions[first_i - 1], glm::vec3(), glm::vec2(), first_i - 1, indices, index_hash);
						process_vertex(vertices, positions[second_i - 1], glm::vec3(), glm::vec2(), second_i - 1, indices, index_hash);
						process_vertex(vertices, positions[third_i - 1], glm::vec3(), glm::vec2(), third_i - 1, indices, index_hash);
					}
					else
					{
						emplace_vertex(vertices, vertex(positions[first_i - 1]), indices);
						emplace_vertex(vertices, vertex(positions[second_i - 1]), indices);
						emplace_vertex(vertices, vertex(positions[third_i - 1]), indices);
					}
				}
				else
				{
					if (third_u == -1)
					{
						if (smooth)
						{
							process_vertex(vertices, positions[first_i - 1], normals[first_n - 1], glm::vec2(), first_i - 1, indices, index_hash);
							process_vertex(vertices, positions[second_i - 1], normals[second_n - 1], glm::vec2(), second_i - 1, indices, index_hash);
							process_vertex(vertices, positions[third_i - 1], normals[third_n - 1], glm::vec2(), third_i - 1, indices, index_hash);
						}
						else
						{
							emplace_vertex(vertices, vertex(positions[first_i - 1], normals[first_n - 1]), indices);
							emplace_vertex(vertices, vertex(positions[second_i - 1], normals[second_n - 1]), indices);
							emplace_vertex(vertices, vertex(positions[third_i - 1], normals[third_n - 1]), indices);
						}
					}
					else
					{
						if (smooth)
						{
							process_vertex(vertices, positions[first_i - 1], normals[first_n - 1], uv_coords[first_u - 1], first_i - 1, indices, index_hash);
							process_vertex(vertices, positions[second_i - 1], normals[second_n - 1], uv_coords[second_u - 1], second_i - 1, indices, index_hash);
							process_vertex(vertices, positions[third_i - 1], normals[third_n - 1], uv_coords[third_u - 1], third_i - 1, indices, index_hash);
						}
						else
						{
							emplace_vertex(vertices, vertex(positions[first_i - 1], normals[first_n - 1], uv_coords[first_u - 1]), indices);
							emplace_vertex(vertices, vertex(positions[second_i - 1], normals[second_n - 1], uv_coords[second_u - 1]), indices);
							emplace_vertex(vertices, vertex(positions[third_i - 1], normals[third_n - 1], uv_coords[third_u - 1]), indices);
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

	vertex_count_ = vertices.size() - vertices_index_;
	index_count_ = indices.size() - indices_index_;

#ifdef DEBUG_LIMIT_INDICES
	vertex_count_ = DEBUG_LIMIT_INDICES;
	index_count_ = DEBUG_LIMIT_INDICES;
#endif

	model_matrix_ = glm::mat4(1.0f);
	orientation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	position_ = glm::vec4(0.0f);
	scale_ = glm::vec3(1.0f);
}

int model::get_index() const
{
	return vertices_index_;
}

int model::transform_vertices_to(vertex *curr, vertex *pos, const int index)
{
	model_matrix_ = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(position_)), scale_);

	for (unsigned int i = 0; i < vertex_count_; ++i)
	{
		*pos = *curr;

		pos->set_position(orientation_ * (model_matrix_ * curr->get_position()));
		pos->set_normal(orientation_ * curr->get_normal());

		++pos;
		++curr;
	}

	return index + vertex_count_;
}

void model::rotate(const glm::vec3& axis, const float angle)
{
	orientation_ = glm::rotate(orientation_, angle, axis);
}

void model::rotate(const float x, const float y, const float z, const float angle)
{
	rotate(glm::vec3(x, y, z), angle);
}

void model::translate(const glm::vec4& offset)
{
	position_ += offset;
}

void model::translate(const float dx, const float dy, const float dz)
{
	position_.x += dx;
	position_.y += dy;
	position_.z += dz;
}

void model::scale(const glm::vec3& ratio)
{
	scale_ = glm::vec3(ratio.x * scale_.x, ratio.y * scale_.y, ratio.z * scale_.z);
}

void model::scale(const float ratio)
{
	scale_ *= ratio;
}

void model::assign_position(const glm::vec4& position)
{
	position_ = position;
}

void model::assign_position(const float x, const float y, const float z)
{
	position_ = glm::vec4(x, y, z, 1);
}

void model::assign_orientation(const glm::quat& orientation)
{
	orientation_ = orientation;
}

void model::assign_orientation(const float x, const float y, const float z, const float w)
{
	orientation_ = glm::quat(w, x, y, z);
}

void model::assign_scale(const glm::vec3& scale)
{
	scale_ = scale;
}

void model::assign_scale(const float x, const float y, const float z)
{
	scale_ = glm::vec3(x, y, z);
}

unsigned model::get_vertex_count() const
{
	return vertex_count_;
}

void model::rotate(const glm::vec3&& axis, const float angle)
{
	rotate(axis, angle);
}

unsigned short model::hash_3(const vertex& v)
{
	const auto pos = v.get_position();

	const auto p1 = 3;
	const auto p2 = 7;
	const auto p3 = 13;

	return static_cast<unsigned short>(floor(pos.x * p1 + pos.y * p2 + pos.z * p3)) % 100; // TODO
}

unsigned short model::emplace_vertex(std::vector<vertex>& vertices, const vertex& v, std::vector<unsigned short>& indices)
{
	vertices.push_back(v);
	indices.push_back(vertices.size() - 1);

	return vertices.size() - 1;
}

void model::process_vertex(std::vector<vertex>& vertices, const glm::vec4& pos, const glm::vec3& norm, const glm::vec2& uv, unsigned int id, 
	std::vector<unsigned short>& indices, std::map<unsigned int, std::map<unsigned short, std::pair<int, std::vector<unsigned short>>>>& index_hash)
{
	auto v = vertex(pos, norm, uv);

	const auto hash = hash_3(v);
	const auto it1 = index_hash.find(hash);
	if (it1 == index_hash.end())
	{
		const auto index = emplace_vertex(vertices, v, indices);

		index_hash[hash] = std::map<unsigned short, std::pair<int, std::vector<unsigned short>>>();
		//index_hash[hash][id] = std::pair<int, std::vector<unsigned short>>();
		index_hash[hash][id].first = 1;
		index_hash[hash][id].second.push_back(index);
	}
	else
	{
		const auto it2 = index_hash[hash].find(id);
		if (it2 == index_hash[hash].end())
		{
			const auto index = emplace_vertex(vertices, v, indices);

			//index_hash[hash][id] = std::pair<int, std::vector<unsigned short>>();
			//index_hash[hash][id].second = std::vector<unsigned short>();
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

int model::find_match(std::vector<vertex>& vertices, std::vector<unsigned short>& indices, const vertex& v)
{
	for (size_t i = 0; i < indices.size(); ++i)
	{
		if (compare_vertices(vertices[indices[i]], v))
			return i;
	}

	return -1;
}

void model::interpolate_normals(std::vector<vertex>& vertices, std::vector<unsigned short>& indices, const int weight,
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

scene::scene()
{
	pck_ptr_ = std::make_unique<std::map<int, material_pack>>();
	use_material(-1);
}

unsigned int scene::load_model(const std::string& filename_model, bool smooth, int mat_id)
{
	if (pck_ptr_->find(mat_id) != pck_ptr_->end())
	{
		(*pck_ptr_)[mat_id].models_.emplace_back((*pck_ptr_)[mat_id].vertices_, (*pck_ptr_)[mat_id].indices_, filename_model, smooth, mat_id);
		return (*pck_ptr_)[mat_id].models_[(*pck_ptr_)[mat_id].models_.size() - 1].get_vertex_count();
	}
	// TODO: do this in the material pack structure
}

void scene::update()
{
	if (pck_ptr_->size() > 1)
	{
		if (!(*pck_ptr_)[0].models_.empty())
		{
			(*pck_ptr_)[0].models_[0].rotate(1, 0, 0, 0.02f);
			(*pck_ptr_)[0].models_[0].assign_position(-0.6, 0, 0);
			(*pck_ptr_)[0].models_[0].assign_scale(0.4, 0.4, 0.4);
			if (pck_ptr_->size() > 2 && !(*pck_ptr_)[1].models_.empty())
			{
				(*pck_ptr_)[1].models_[0].assign_position(0.6, 0, 0);
				(*pck_ptr_)[1].models_[0].assign_scale(0.4, 0.4, 0.4);
			}
		}
	}
}

void scene::use_material(const int mat_id)
{
	(*pck_ptr_)[mat_id] = material_pack();
}

void scene::disable_material(int mat_id)
{
	pck_ptr_->erase(mat_id);
}

const std::vector<vertex>& scene::get_vertices(const int mat_id)
{
	(*pck_ptr_)[mat_id].transformed_vertices_.resize((*pck_ptr_)[mat_id].vertices_.size());
#ifdef DEBUG_LIMIT_INDICES
	transformed_vertices_.resize(DEBUG_LIMIT_INDICES);
#endif
	int index = 0;
	for (auto && model : (*pck_ptr_)[mat_id].models_)
	{
		index = model.transform_vertices_to(&(*pck_ptr_)[mat_id].vertices_[model.get_index()], &(*pck_ptr_)[mat_id].transformed_vertices_[index], index);
	}

	return (*pck_ptr_)[mat_id].transformed_vertices_;
}

const std::vector<unsigned short>& scene::get_indices(const int mat_id) const
{
	return (*pck_ptr_)[mat_id].indices_;
}
