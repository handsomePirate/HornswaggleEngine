#include "RenderHelper.hpp"
#include "Scene.hpp"

model_handle::model_handle(model_instance *mi)
	: mi_(mi)
{
	if (mi_)
		mi_->register_handle(this);
}

void model_handle::rotate(const glm::vec3& axis, const float angle) const
{
	if (!mi_)
		return;
	mi_->rotate(axis, angle);
}

void model_handle::rotate(const float x, const float y, const float z, const float angle) const
{
	if (!mi_)
		return;
	mi_->rotate(x, y, z, angle);
}

void model_handle::translate(const glm::vec4& offset) const
{
	if (!mi_)
		return;
	mi_->translate(offset);
}

void model_handle::translate(const float dx, const float dy, const float dz) const
{
	if (!mi_)
		return;
	mi_->translate(dx, dy, dz);
}

void model_handle::scale(const glm::vec3& ratio) const
{
	if (!mi_)
		return;
	mi_->scale(ratio);
}

void model_handle::scale(const float ratio) const
{
	if (!mi_)
		return;
	mi_->scale(ratio);
}

void model_handle::assign_position(const glm::vec4& position) const
{
	if (!mi_)
		return;
	mi_->assign_position(position);
}

void model_handle::assign_position(const float x, const float y, const float z) const
{
	if (!mi_)
		return;
	mi_->assign_position(x, y, z);
}

void model_handle::assign_orientation(const glm::quat& orientation) const
{
	if (!mi_)
		return;
	mi_->assign_orientation(orientation);
}

void model_handle::assign_orientation(const float x, const float y, const float z, const float w) const
{
	if (!mi_)
		return;
	mi_->assign_orientation(x, y, z, w);
}

void model_handle::assign_scale(const glm::vec3& scale) const
{
	if (!mi_)
		return;
	mi_->assign_scale(scale);
}

void model_handle::assign_scale(const float x, const float y, const float z) const
{
	if (!mi_)
		return;
	mi_->assign_scale(x, y, z);
}

void model_handle::delete_instance_reference()
{
	mi_ = nullptr;
}
