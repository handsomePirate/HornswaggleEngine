#include "VulkanInstance.hpp"
#include "Utils.hpp"
#include <vulkan/vulkan_win32.h>

Vulkan::ApplicationInfo::ApplicationInfo(
	const std::string& applicationName, unsigned int applicationVersion, 
	const std::string& engineName, unsigned int engineVersion, 
	unsigned int vulkanMajor, unsigned int vulkanMinor
)
{
	info_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info_.pNext = NULL;
	info_.pApplicationName = CopyChars(applicationName);
	info_.applicationVersion = applicationVersion;
	info_.pEngineName = CopyChars(engineName);
	info_.engineVersion = engineVersion;
	info_.apiVersion = VK_MAKE_VERSION(vulkanMajor, vulkanMinor, 0);
}

Vulkan::ApplicationInfo::~ApplicationInfo()
{
	delete[] info_.pApplicationName;
	delete[] info_.pEngineName;
}

const VkApplicationInfo& Vulkan::ApplicationInfo::Get() const
{
	return info_;
}

Vulkan::Instance::Instance(const VkApplicationInfo& applicationInfo)
{
	std::vector<const char *> extNames;
	extNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

	VkInstanceCreateInfo inst_info = {};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = NULL;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &applicationInfo;
	inst_info.enabledExtensionCount = extNames.size();
	inst_info.ppEnabledExtensionNames = extNames.data();
	inst_info.enabledLayerCount = 0;
	inst_info.ppEnabledLayerNames = NULL;

	VkResult result = vkCreateInstance(&inst_info, NULL, &instance_);
	assert(!result);

	// TODO: handle errors
}

Vulkan::Instance::~Instance()
{
	vkDestroyInstance(instance_, NULL);
}

const VkInstance& Vulkan::Instance::Get() const
{
	return instance_;
}
