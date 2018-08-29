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

void camera::set_focus(glm::vec3& f)
{
	focus_ = f;
	
	create_view_matrix();
}

void camera::set_forward(glm::vec3& f)
{
	focus_ = glm::vec3(position_) + normalize(f);

	create_view_matrix();
}

void camera::set_position(glm::vec3& p)
{
	position_ = glm::vec4(p, 1);

	create_view_matrix();
}

void camera::set_up(glm::vec3& u)
{
	up_ = u;

	create_view_matrix();
}

void camera::set_position_forward(glm::vec3& p, glm::vec3& f)
{
	position_ = glm::vec4(p, 1);
	focus_ = p + normalize(f);

	create_view_matrix();
}

void camera::set_position_focus(glm::vec3& p, glm::vec3& f)
{
	position_ = glm::vec4(p, 1);;
	focus_ = f;
	
	create_view_matrix();
}

void camera::set_position_up(glm::vec3& p, glm::vec3& u)
{
	position_ = glm::vec4(p, 1);
	up_ = u;

	create_view_matrix();
}

void camera::set_forward_up(glm::vec3& f, glm::vec3& u)
{
	focus_ = glm::vec3(position_) + normalize(f);
	up_ = u;

	create_view_matrix();
}

void camera::set_focus_up(glm::vec3& f, glm::vec3& u)
{
	focus_ = f;
	up_ = u;

	create_view_matrix();
}

void camera::set_transform_forward(glm::vec3& p, glm::vec3& f, glm::vec3& u)
{
	position_ = glm::vec4(p, 1);
	focus_ = p + normalize(f);
	up_ = u;

	create_view_matrix();
}

void camera::set_transform_focus(glm::vec3& p, glm::vec3& f, glm::vec3& u)
{
	position_ = glm::vec4(p, 1);
	focus_ = f;
	up_ = u;

	create_view_matrix();
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

	create_projection_matrix();
}

void camera::set_near_far(const float z_near, const float z_far)
{
	z_near_ = z_near;
	z_far_ = z_far;

	create_projection_matrix();
}

void camera::set_near(const float z_near)
{
	z_near_ = z_near;

	create_projection_matrix();
}

void camera::set_far(const float z_far)
{
	z_far_ = z_far;

	create_projection_matrix();
}

void camera::set_fov(const float fov)
{
	fov_ = fov;

	create_projection_matrix();
}

void camera::set_frustum(float fov, float aspect, float z_near, float z_far)
{
	fov_ = fov;
	aspect_ = aspect;
	z_near_ = z_near;
	z_far_ = z_far;

	create_projection_matrix();
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

void camera::create_projection_matrix()
{
	projection_matrix_ = glm::perspective(fov_, aspect_, z_near_, z_far_);
	/*
	if (z_near_ < z_far_ && z_near_ > 0 && aspect_ > 0 && fov_ <= PI && fov_ > 0)
	{
		const float x = 1.0f / static_cast<float>(tan(0.5f * fov_));
		const float y = x * aspect_;

		const float zm = z_far_ - z_near_;

		const float a = -(z_far_ + z_near_) / zm;
		const float b = -2 * z_far_ * z_near_ / zm;

		projection_matrix_ = glm::mat4(
			x, 0,  0, 0,
			0, y,  0, 0,
			0, 0,  a, b,
			0, 0, -1, 0
		);

		//projection_matrix_ = transpose(projection_matrix_);
	}
	*/
}

void camera::create_view_matrix()
{
	view_matrix_ = lookAt(glm::vec3(position_), glm::vec3(focus_), glm::vec3(up_));
	/*
	const auto target = normalize(position_ + forward_);
	const auto dir = normalize(target - position_);
	const auto aside = cross(dir, up_);
	const auto cam_up = cross(aside, dir);

	view_matrix_ = inverse(
		glm::mat4(
			aside.x, cam_up.x, dir.x, -position_.x,
			aside.y, cam_up.y, dir.y, -position_.y,
			aside.z, cam_up.z, dir.z, -position_.z,
			0,		 0,		   0,	   1
		)
	);
	*/
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

model::model(std::vector<vertex>& vertices, std::vector<unsigned short>& indices, const std::string& filename_model, const std::string& filename_texture)
{
	std::ifstream ifs(filename_model);

	std::string line;
	std::string sign;

	const int vert_index = vertices.size();
	const int ind_index = indices.size();

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

	vertex_count_ = vertices.size() - vert_index;
	index_count_ = indices.size() - ind_index;

#ifdef DEBUG_LIMIT_INDICES
	vertex_count_ = DEBUG_LIMIT_INDICES;
	index_count_ = DEBUG_LIMIT_INDICES;
#endif

	if (vertex_count_ > 0)
		vertices_ = &vertices[vert_index];
	if (index_count_ > 0)
		indices_ = &indices[ind_index];

	model_matrix_ = glm::mat4(1.0f);
	orientation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	position_ = glm::vec4(0.0f);
	scale_ = glm::vec3(1.0f);

	if (filename_texture.empty())
	{
		tex_ptr_.reset();
		return;
	}
	tex_ptr_ = std::make_unique<texture>(filename_texture);
}

int model::transform_vertices_to(vertex *pos, const int index)
{
	vertex *curr = vertices_;

	model_matrix_ = mat4_cast(orientation_) * glm::translate(glm::scale(glm::mat4(1.0f), scale_), glm::vec3(position_));

	for (unsigned int i = 0; i < vertex_count_; ++i)
	{
		*pos = *curr;

		pos->set_position(model_matrix_ * curr->get_position());

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

void model::rotate(const glm::vec3&& axis, const float angle)
{
	rotate(axis, angle);
}

void scene::load_model(const std::string& filename_model, const std::string& filename_texture)
{
	models_.emplace_back(vertices_, indices_, filename_model, filename_texture);
}

void scene::update()
{
	if (!models_.empty())
	{
		models_[0].rotate(1, 0, 0, 0.02f);
	}
}

const std::vector<vertex>& scene::get_vertices()
{
	transformed_vertices_.resize(vertices_.size());
#ifdef DEBUG_LIMIT_INDICES
	transformed_vertices_.resize(DEBUG_LIMIT_INDICES);
#endif
	int index = 0;
	for (auto && model : models_)
	{
		index = model.transform_vertices_to(&transformed_vertices_[index], index);
	}

	return transformed_vertices_;
}

const std::vector<unsigned short>& scene::get_indices() const
{
	return indices_;
}
