#pragma once
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>
#include <thread>

#include "VulkanInstance.hpp"

struct Window
{
	VkSurfaceKHR surface;
	HWND hwnd;
};

struct WindowProvider
{
	static Window GetVulkanWindow(const Vulkan::Instance& instance, unsigned int sizeX, unsigned int sizeY);
	static void DestroyVulkanWindow(const Vulkan::Instance& instance, Window& window);
private:
	static HWND GetWin32WindowHandle(HINSTANCE HIn, unsigned int sizeX, unsigned int sizeY);
};
