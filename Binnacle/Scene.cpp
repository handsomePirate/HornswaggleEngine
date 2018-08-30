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
#include <iostream>
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
	const auto forward = normalize(focus_ - glm::vec3(position_));
	const auto offset_angle = acos(dot(glm::vec3(0.0f, 0.0f, -1.0f), forward) / length(forward));
	const auto offset_axis = normalize(cross(glm::vec3(0.0f, 0.0f, -1.0f), forward));

	if (offset_axis.x != offset_axis.x || offset_axis.y != offset_axis.y || offset_axis.z != offset_axis.z)
	{
		rotate(offset_axis, offset_angle);
		return;
	}

	const glm::quat offset_rotation = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), offset_angle, offset_axis);
	const auto new_axis = offset_rotation * axis;
	rotate(offset_rotation * axis, angle);
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
	const auto forward = normalize(focus_ - glm::vec3());
	const auto offset_angle = acos(dot(glm::vec3(0.0f, 0.0f, -1.0f), forward) / length(forward));
	const auto offset_axis = normalize(cross(glm::vec3(0.0f, 0.0f, -1.0f), forward));

	if (offset_axis.x != offset_axis.x || offset_axis.y != offset_axis.y || offset_axis.z != offset_axis.z)
	{
		rotate(offset_axis, offset_angle);
		return;
	}

	const glm::quat offset_rotation = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), offset_angle, offset_axis);
	const glm::vec3 real_offset = offset_rotation * offset;

	translate(real_offset);
}

void camera::translate_local(const float dx, const float dy, const float dz)
{
	translate_local(glm::vec3(dx, dy, dz));
}

void camera::translate_local_2_d(const glm::vec3& offset)
{
	const auto forward = normalize(focus_ - glm::vec3(position_));
	const auto offset_angle = acos(dot(glm::vec3(0.0f, 0.0f, -1.0f), forward) / length(forward));
	const auto offset_axis = normalize(cross(glm::vec3(0.0f, 0.0f, -1.0f), forward));

	if (offset_axis.x != offset_axis.x || offset_axis.y != offset_axis.y || offset_axis.z != offset_axis.z)
	{
		rotate(offset_axis, offset_angle);
		return;
	}

	auto length = glm::length(offset);
	const glm::quat offset_rotation = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), offset_angle, offset_axis);
	glm::vec3 real_offset = offset_rotation * offset;
	real_offset.y = 0.0f;
	real_offset = normalize(real_offset) * length;

	translate(real_offset);
}

void camera::translate_local_2_d(const float dx, const float dy, const float dz)
{
	translate_local_2_d(glm::vec3(dx, dy, dz));
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

environment::environment(camera& cam)
	: camera_(cam) { }

camera& environment::get_camera()
{
	return camera_;
}

const std::vector<glm::vec3>& environment::get_lights() const
{
	return lights_;
}

#define DEBUG_LIMIT_INDICES_ 3

model::model(std::vector<vertex>& vertices, std::vector<unsigned short>& indices, const std::string& filename_model, const int mat_id)
{
	std::ifstream ifs(filename_model);

	std::string line;
	std::string sign;

	vertices_index_ = vertices.size();
	indices_index_ = indices.size();

	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> positions;
	std::vector<glm::vec2> uv_coords;

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
					vertices.emplace_back(positions[first_i - 1]);
					indices.push_back(vertices.size() - 1);
					vertices.emplace_back(positions[second_i - 1]);
					indices.push_back(vertices.size() - 1);
					vertices.emplace_back(positions[third_i - 1]);
					indices.push_back(vertices.size() - 1);
				}
				else
				{
					if (third_u == -1)
					{
						vertices.emplace_back(positions[first_i - 1], normals[first_n - 1]);
						indices.push_back(vertices.size() - 1);
						vertices.emplace_back(positions[second_i - 1], normals[second_n - 1]);
						indices.push_back(vertices.size() - 1);
						vertices.emplace_back(positions[third_i - 1], normals[third_n - 1]);
						indices.push_back(vertices.size() - 1);
					}
					else
					{
						vertices.emplace_back(positions[first_i - 1], normals[first_n - 1], uv_coords[first_u - 1]);
						indices.push_back(vertices.size() - 1);
						vertices.emplace_back(positions[second_i - 1], normals[second_n - 1], uv_coords[second_u - 1]);
						indices.push_back(vertices.size() - 1);
						vertices.emplace_back(positions[third_i - 1], normals[third_n - 1], uv_coords[third_u - 1]);
						indices.push_back(vertices.size() - 1);
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

void model::rotate(const glm::vec3&& axis, const float angle)
{
	rotate(axis, angle);
}

scene::scene()
{
	pck_ptr_ = std::make_unique<std::map<int, material_pack>>();
	use_material(-1);
}

void scene::load_model(const std::string& filename_model, int mat_id)
{
	if (pck_ptr_->find(mat_id) != pck_ptr_->end())
		(*pck_ptr_)[mat_id].models_.emplace_back((*pck_ptr_)[mat_id].vertices_, (*pck_ptr_)[mat_id].indices_, filename_model, mat_id);
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
