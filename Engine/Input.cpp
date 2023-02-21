#include "stdafx.h"
#include "Input.h"

#include <GLFW/glfw3.h>

void Input::Init()
{

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
	double x, y;
	glfwGetCursorPos(Application::window, &x, &y);

	return XMFLOAT2(x, y);
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
