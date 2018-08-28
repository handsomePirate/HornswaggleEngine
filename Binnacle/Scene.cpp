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
	: position_(position), focus_(focus), up_(normalize(up)), fov_(PI * (fov / 180.0f)), aspect_(aspect), z_near_(z_near), z_far_(z_far)
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
	focus_ = position_ + normalize(f);

	create_view_matrix();
}

void camera::set_position(glm::vec3& p)
{
	position_ = p;

	create_view_matrix();
}

void camera::set_up(glm::vec3& u)
{
	up_ = u;

	create_view_matrix();
}

void camera::set_position_forward(glm::vec3& p, glm::vec3& f)
{
	position_ = p;
	focus_ = position_ + normalize(f);

	create_view_matrix();
}

void camera::set_position_focus(glm::vec3& p, glm::vec3& f)
{
	position_ = p;
	focus_ = f;
	
	create_view_matrix();
}

void camera::set_position_up(glm::vec3& p, glm::vec3& u)
{
	position_ = p;
	up_ = u;

	create_view_matrix();
}

void camera::set_forward_up(glm::vec3& f, glm::vec3& u)
{
	focus_ = position_ + normalize(f);
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
	position_ = p;
	focus_ = position_ + normalize(f);
	up_ = u;

	create_view_matrix();
}

void camera::set_transform_focus(glm::vec3& p, glm::vec3& f, glm::vec3& u)
{
	position_ = p;
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

const glm::vec3& camera::get_position() const
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
	view_matrix_ = lookAt(position_, focus_, up_);
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

#define DEBUG_LIMIT_INDICES_ 6

scene::scene(const std::string& filename)
{
	std::ifstream ifs(filename);

	std::string line;
	std::string sign;
	
	while (ifs.good())
	{		
		if (ifs.peek() == 'v')
		{
			ifs >> sign;
			if (sign == "v")
			{
				float x, y, z;
				ifs >> x; ifs >> y; ifs >> z;
				vertices_.emplace_back(x, y, z);
			}
			continue;
		}
		if (ifs.peek() == 'f')
		{
			ifs >> sign;
			int first, second, third;
			ifs >> first;
			if (ifs.peek() == '/')
			{
				while (ifs.get() != ' '){}
			}
			ifs >> second;
			if (ifs.peek() == '/')
			{
				while (ifs.get() != ' ') {}
			}
			ifs >> third;
#ifdef DEBUG_LIMIT_INDICES
			if (indices_.size() < DEBUG_LIMIT_INDICES)
			{
#endif
				indices_.push_back(first - 1);
				indices_.push_back(second - 1);
				indices_.push_back(third - 1);
#ifdef DEBUG_LIMIT_INDICES
			}
#endif
			continue;
		}

		std::getline(ifs, line);
		// TODO: improve
	}

	camera_ = glm::vec3(0, 0, -1);
	
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
}

const std::vector<glm::vec3>& scene::get_vertices() const
{
	return vertices_;
}

const std::vector<unsigned short>& scene::get_indices() const
{
	return indices_;
}
