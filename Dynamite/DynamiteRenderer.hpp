#pragma once
#include "VulkanDevice.hpp"
#include "WindowProvider.hpp"

namespace Dynamite
{
	struct Renderer
	{
		Renderer(const std::string& engineName, unsigned int engineVersion);
		~Renderer();
		Window& SetTargetWindow(unsigned int sizeX, unsigned int sizeY);
		void Introduce();
		void Render();
	private:
		void createCommandPool();

		Vulkan::ApplicationInfo aplicationInfo_;
		Vulkan::Instance vulkanInstance_;
		Vulkan::Device device_;
		const unsigned int commandBufferCount_ = 1;
		std::vector<VkCommandBuffer> commandBuffers_;
		VkCommandPool commandPool_;
		Window window_;

		enum RenderTargetType
		{
			NONE,
			WINDOW,
			IMAGE
		} renderTargetType;
	};
}
