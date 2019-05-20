#pragma once
#include "VulkanInstance.hpp"
#include "WindowProvider.hpp"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>

namespace Vulkan
{
	struct Device
	{
		Device();
		Device(const Instance& instance, const Window& window)
			: Device(instance.Get(), window) {}
		Device(const VkInstance& instance, const Window& window);
		~Device();
		const VkDevice& Get() const;
		const VkPhysicalDeviceProperties& GetProperties() const;
		unsigned int GetFamilyIndex() const;
		const VkPhysicalDevice GetPhysicalDevice() const;
		bool Valid() const;
	private:
		bool CheckDevicePertinence(const VkPhysicalDevice& device, const VkSurfaceKHR& surface, VkDeviceQueueCreateInfo& queue, bool& present) const;

		VkDevice device_;
		VkPhysicalDevice physicalDevice_;
		VkPhysicalDeviceProperties physicalDeviceProperties_;
		unsigned int queueFamilyIndex_;
	};
}