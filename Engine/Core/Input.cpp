#include "stdafx.h"
#include "Input.h"

#include <GLFW/glfw3.h>

void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	scrolled = true;
	scrollValue = yoffset;
}

void Input::Init()
{
	glfwSetScrollCallback(Application::window, ScrollCallback);
}

void Input::Update()
{
	prevKeyMap = keyMap;

	for (size_t i = 0; i < GLFW_KEY_LAST; i++)
	{
		keyMap[i] = GetKey(i);
	}

	prevMouseMap = mouseMap;

	for (size_t i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
	{
		mouseMap[i] = GetMouseButton(i);
	}

	if (scrolled) scrolled = false;
	else scrollValue = 0.0f;

	lastMousePos = mousePos;

	double x, y;
	glfwGetCursorPos(Application::window, &x, &y);
	mousePos = XMFLOAT2(x, y);
}

void Input::CleanUp()
{
	glfwSetScrollCallback(Application::window, nullptr);
}

bool Input::GetKey(int key)
{
	return glfwGetKey(Application::window, key);
}

bool Input::GetKeyDown(int key)
{
	return keyMap[key] && !prevKeyMap[key];
}

bool Input::GetKeyUp(int key)
{
	return !keyMap[key] && prevKeyMap[key];
}

XMFLOAT2 Input::GetMousePosition()
{
	return mousePos;
}

bool Input::GetMouseButton(int button)
{
	return glfwGetMouseButton(Application::window, button);
}

bool Input::GetMouseButtonDown(int button)
{
	return mouseMap[button] && !prevMouseMap[button];
}

bool Input::GetMouseButtonUp(int button)
{
	return !mouseMap[button] && prevMouseMap[button];
}

float Input::GetMouseScrollDelta()
{
	return scrollValue;
}

XMFLOAT2 Input::GetMousePosDelta()
{
	return XMFLOAT2(mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y);
}
