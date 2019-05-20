#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>
#include <string>

namespace Vulkan
{
	struct ApplicationInfo
	{
		ApplicationInfo(
			const std::string& applicationName, unsigned int applicationVersion,
			const std::string& engineName, unsigned int engineVersion,
			unsigned int vulkanMajor, unsigned int vulkanMinor
		);
		~ApplicationInfo();
		const VkApplicationInfo& Get() const;
	private:
		VkApplicationInfo info_;
	};

	struct Instance
	{
		explicit Instance(const ApplicationInfo& applicationInfo)
			: Instance(applicationInfo.Get()) {};
		explicit Instance(const VkApplicationInfo& applicationInfo);
		~Instance();
		const VkInstance& Get() const;
	private:
		VkInstance instance_;
		VkResult result_;
	};
}
