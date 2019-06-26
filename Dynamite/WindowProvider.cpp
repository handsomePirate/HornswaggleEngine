#include "WindowProvider.hpp"

Window WindowProvider::GetVulkanWindow(const Vulkan::Instance& instance, const unsigned int sizeX, const unsigned int sizeY)
{
	VkWin32SurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = NULL;
	createInfo.hinstance = (HINSTANCE)GetModuleHandle(NULL);
	createInfo.hwnd = GetWin32WindowHandle(createInfo.hinstance, sizeX, sizeY);
	Window window;
	window.hwnd = createInfo.hwnd;
	vkCreateDisplayPlaneSurfaceKHR()
	VkResult res = vkCreateWin32SurfaceKHR(instance.Get(), &createInfo, NULL, &window.surface);
	
	assert(!res);

	return window;
}

void WindowProvider::DestroyVulkanWindow(const Vulkan::Instance& instance, Window& window)
{
	if (window.hwnd)
		DestroyWindow(window.hwnd);
	if (window.surface)
		vkDestroySurfaceKHR(instance.Get(), window.surface, NULL);
	window.hwnd = 0;
	window.surface = 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool Register(HINSTANCE HIn);

HWND WindowProvider::GetWin32WindowHandle(HINSTANCE HIn, const unsigned int sizeX, const unsigned int sizeY)
{
	HWND Hw;
	DWORD TCount;

	if (!Register(HIn)) return 0;

	Hw = CreateWindowEx(WS_EX_TOPMOST, "Dynamite", "", WS_OVERLAPPEDWINDOW,
		0, 0, sizeX, sizeY,
		GetDesktopWindow(), NULL, HIn, NULL);

	if (Hw == NULL) return 0;

	//ShowWindow(Hw, SW_SHOW); UpdateWindow(Hw); SetFocus(Hw);

	return Hw;
}

LRESULT CALLBACK WndProc(HWND Hw, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	default: return DefWindowProc(Hw, Msg, wParam, lParam);
	}
}

bool Register(HINSTANCE HIn)
{
	WNDCLASSEX Wc;

	Wc.cbSize = sizeof(WNDCLASSEX);
	Wc.style = 0;
	Wc.lpfnWndProc = WndProc;
	Wc.cbClsExtra = 0;
	Wc.cbWndExtra = 0;
	Wc.hInstance = HIn;
	Wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	Wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	Wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	Wc.lpszMenuName = NULL;
	Wc.lpszClassName = "Dynamite";
	Wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	return RegisterClassEx(&Wc);
}