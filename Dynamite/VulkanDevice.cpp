#include "VulkanDevice.hpp"

#include <vector>

Vulkan::Device::Device()
	: device_(0), physicalDevice_(0), physicalDeviceProperties_(), queueFamilyIndex_(0) {}

Vulkan::Device::Device(const VkInstance& instance, const Window& window)
{
	uint32_t gpuCount = 1;
	VkResult res = vkEnumeratePhysicalDevices(instance, &gpuCount, NULL);
	assert(gpuCount);
	VkPhysicalDevice *gpus = (VkPhysicalDevice *)malloc(gpuCount * sizeof(VkPhysicalDevice));
	res = vkEnumeratePhysicalDevices(instance, &gpuCount, gpus);

	// TODO: handle errors

	bool found = false;
	unsigned int physicalDeviceIndex = 0;
	VkDeviceQueueCreateInfo chosenQueueInfo;
	bool chosenPresent = false;

	for (unsigned int i = 0; i < gpuCount; ++i)
	{
		VkDeviceQueueCreateInfo queueInfo;
		bool present;
		if (CheckDevicePertinence(gpus[i], window.surface, queueInfo, present))
		{
			if (found == false)
			{
				physicalDeviceIndex = i;
				chosenQueueInfo = queueInfo;
				found = true;
				chosenPresent = present;
				vkGetPhysicalDeviceProperties(gpus[physicalDeviceIndex], &physicalDeviceProperties_);
			}
			else
			{
				if (present && !chosenPresent)
				{
					physicalDeviceIndex = i;
					chosenQueueInfo = queueInfo;
					vkGetPhysicalDeviceProperties(gpus[physicalDeviceIndex], &physicalDeviceProperties_);
				}
				else if (present)
				{
					VkPhysicalDeviceProperties physicalDeviceProperties;
					vkGetPhysicalDeviceProperties(gpus[physicalDeviceIndex], &physicalDeviceProperties);

					if (physicalDeviceProperties_.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					{
						physicalDeviceProperties_ = physicalDeviceProperties;
						physicalDeviceIndex = i;
						chosenQueueInfo = queueInfo;
					}
				}
			}
		}
	}

	// TODO: handle no graphics card
	assert(found);
	// TODO: handle no present -- add another queue
	//assert(chosenPresent);

	queueFamilyIndex_ = chosenQueueInfo.queueFamilyIndex;

	float queue_priorities[1] = { 0.0 };
	chosenQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	chosenQueueInfo.pNext = NULL;
	chosenQueueInfo.flags = 0;
	chosenQueueInfo.queueCount = 1;
	chosenQueueInfo.pQueuePriorities = queue_priorities;

	std::vector<const char *> extNames;
	extNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo deviceInfo;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = NULL;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &chosenQueueInfo;
	deviceInfo.enabledExtensionCount = extNames.size();
	deviceInfo.ppEnabledExtensionNames = extNames.data();
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = NULL;
	deviceInfo.pEnabledFeatures = NULL;

	physicalDevice_ = gpus[physicalDeviceIndex];

	// TODO: allocator
	res = vkCreateDevice(physicalDevice_, &deviceInfo, NULL, &device_);
	assert(!res);
	// TODO: handle errors

	delete[] gpus;
}

Vulkan::Device::~Device()
{
	if (Valid())
		vkDestroyDevice(device_, NULL);
}

const VkDevice& Vulkan::Device::Get() const
{
	return device_;
}

const VkPhysicalDeviceProperties& Vulkan::Device::GetProperties() const
{
	return physicalDeviceProperties_;
}

unsigned int Vulkan::Device::GetFamilyIndex() const
{
	return queueFamilyIndex_;
}

const VkPhysicalDevice Vulkan::Device::GetPhysicalDevice() const
{
	return physicalDevice_;
}

bool Vulkan::Device::Valid() const
{
	return device_;
}

bool Vulkan::Device::CheckDevicePertinence(const VkPhysicalDevice& device, const VkSurfaceKHR& surface, VkDeviceQueueCreateInfo& queueInfo, bool& present) const
{
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
	assert(queueFamilyCount);
	VkQueueFamilyProperties *properties = (VkQueueFamilyProperties *)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, properties);

	bool found = false;
	present = false;
	for (unsigned int i = 0; i < queueFamilyCount; ++i) 
	{
		VkBool32 presentSupport = VK_FALSE;
		VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		assert(!res);
		if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
		{
			if (!found)
			{
				queueInfo.queueFamilyIndex = i;
				found = true;
				if (presentSupport == VK_TRUE)
				{
					present = true;
					break;
				}
			}
			else
			{
				if (presentSupport == VK_TRUE)
				{
					queueInfo.queueFamilyIndex = i;
					present = true;
					break;
				}
			}
		}
	}

	delete[] properties;

	return found;
}
