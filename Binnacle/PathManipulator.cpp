#include "PathManipulator.hpp"

path_manipulator::path_manipulator(std::string& root)
	: root_(root)
{ }

void path_manipulator::cd_command(std::string& path, const std::string& parameter) const
{
	if (parameter == "..")
		cut_last(path);
	else if (parameter[0] == '\\')
		path = root_ + parameter.substr(1);
	else
		path += "\\" + parameter;
}

void path_manipulator::cut_last(std::string& path)
{
	unsigned int before_last = 0;
	unsigned int last = 0;
	for (unsigned int i = 0; i < path.size(); ++i)
	{
		if (path[i] == '\\')
		{
			if (last > 0)
				before_last = last;
			last = i + 1;
		}
	}
	if (last > 0)
	{
		if (last < path.size())
			path = path.substr(0, last);
		else
		{
			if (before_last > 0)
				path = path.substr(0, before_last);
			else
				path = "";
		}
	}
	else
		path = "";
}

void path_manipulator::cut_first(std::string& path)
{
	for (unsigned int i = 0; i < path.size(); ++i)
	{
		if (path[i] == '\\')
		{
			const auto k = i + 1;
			path = path.substr(k);
			return;
		}
	}
}

void path_manipulator::register_path(std::string& name, std::string& path_from_root)
{
	paths_.insert_or_assign(std::move(name), std::move(path_from_root));
}

std::string path_manipulator::get_full_path(const std::string& name)
{
	if (paths_.find(name) == paths_.end())
		return "";

	auto path = paths_[name];
	auto root = root_;
	while (path[0] == '.' && path[1] == '.')
	{
		cut_first(path);
		cut_last(root);
	}

	return root + path + '\\';
}
