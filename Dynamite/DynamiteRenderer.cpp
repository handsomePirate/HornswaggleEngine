#include "DynamiteRenderer.hpp"

#include <iostream>

Dynamite::Renderer::Renderer(const std::string& engineName, const unsigned int engineVersion)
	: aplicationInfo_("Dynamite renderer", 1, engineName, engineVersion, 1, 0), vulkanInstance_(aplicationInfo_), renderTargetType(NONE) {}

Dynamite::Renderer::~Renderer()
{
	vkDestroyCommandPool(device_.Get(), commandPool_, NULL);
	WindowProvider::DestroyVulkanWindow(vulkanInstance_, window_);
}

Window& Dynamite::Renderer::SetTargetWindow(unsigned int sizeX, unsigned int sizeY)
{
	renderTargetType = WINDOW;
	window_ = WindowProvider::GetVulkanWindow(vulkanInstance_, sizeX, sizeY);
	device_ = Vulkan::Device(vulkanInstance_, window_);

	createCommandPool();

	return window_;
}

void Dynamite::Renderer::Introduce() const
{
	if (!device_.Valid())
		return;
	std::cout << aplicationInfo_.Get().pEngineName << " v" << aplicationInfo_.Get().engineVersion << " using " 
		<< aplicationInfo_.Get().pApplicationName << " v" << aplicationInfo_.Get().applicationVersion << "..." << std::endl;
	std::cout << "Graphics device: " << device_.GetProperties().deviceName << std::endl;
}

void Dynamite::Renderer::Render()
{
	if (renderTargetType == NONE)
		return;
	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = NULL;
	commandBufferBeginInfo.pInheritanceInfo = NULL;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffers_[0], &commandBufferBeginInfo);

	vkEndCommandBuffer(commandBuffers_[0]);
}

void Dynamite::Renderer::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = NULL;
	commandPoolInfo.queueFamilyIndex = device_.GetFamilyIndex();
	commandPoolInfo.flags = 0; //VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT

	commandPool_ = nullptr;
	VkCommandPool command_pool;
	const VkDevice device = device_.Get();
	VkResult res = vkCreateCommandPool(device, &commandPoolInfo, NULL, &command_pool);
	assert(!res);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = NULL;
	commandBufferAllocateInfo.commandPool = commandPool_;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = commandBufferCount_;

	commandBuffers_.resize(commandBufferCount_);
	res = vkAllocateCommandBuffers(device_.Get(), &commandBufferAllocateInfo, commandBuffers_.data());
	assert(!res);

	// TODO: handle errors
}
