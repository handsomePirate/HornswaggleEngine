#pragma once
#include <string>
#include <unordered_map>

struct path_manipulator
{
	explicit path_manipulator(std::string& root);
	void cd_command(std::string& path, const std::string& parameter) const;
	static void cut_last(std::string& path);
	static void cut_first(std::string& path);

	void register_path(std::string& name, std::string& path_from_root);
	std::string get_full_path(const std::string& name);
private:
	std::string root_;
	std::unordered_map<std::string, std::string> paths_;
};
