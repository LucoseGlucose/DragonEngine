#include "stdafx.h"
#include "CameraControllerComponent.h"

#include "Input.h"

CameraControllerComponent::CameraControllerComponent(SceneObject* owner) : Component(owner)
{

}

void CameraControllerComponent::OnUpdate()
{
	float scrollDelta = Input::GetMouseScrollDelta();

	if (scrollDelta != 0.f)
	{
		Vector3 newPos = GetTransform()->GetPosition() + GetTransform()->GetForward() * scrollDelta * scrollSpeed;
		GetTransform()->SetPosition(newPos);
	}

	Vector2 posDelta = XMVector2ClampLength(Input::GetMousePosDelta(), 0.f, 10.f);

	if (Input::GetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE))
	{
		Vector3 newPos = GetTransform()->GetPosition() + GetTransform()->GetRight() * -posDelta.x * panSpeed * TimeManager::GetDeltaTime()
			+ GetTransform()->GetUp() * posDelta.y * panSpeed * TimeManager::GetDeltaTime();

		GetTransform()->SetPosition(newPos);
	}

	if (Input::GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
	{
		glfwSetInputMode(Application::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		float yaw = posDelta.x * (float)TimeManager::GetDeltaTime() * rotateSpeed;
		yaw = DirectX::XMConvertToRadians(yaw);

		float pitch = posDelta.y * (float)TimeManager::GetDeltaTime() * rotateSpeed;
		pitch = DirectX::XMConvertToRadians(pitch);

		XMFLOAT4 rot = GetTransform()->GetRotation();
		XMVECTOR rotVec = DirectX::XMLoadFloat4(&rot);

		XMVECTOR yawVec = DirectX::XMQuaternionMultiply(rotVec, XMQuaternionRotationAxis(DirectX::g_XMIdentityR1.v, yaw));

		XMFLOAT4 yawRot;
		DirectX::XMStoreFloat4(&yawRot, yawVec);

		GetTransform()->SetRotation(yawRot);

		Vector3 right = GetTransform()->GetRight();
		XMVECTOR pitchVec = DirectX::XMQuaternionMultiply(yawVec, XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&right), pitch));

		XMFLOAT4 finalRot;
		DirectX::XMStoreFloat4(&finalRot, pitchVec);

		GetTransform()->SetRotation(finalRot);

		if (Input::GetKey(GLFW_KEY_W))
		{
			Vector3 newPos = GetTransform()->GetPosition() + GetTransform()->GetForward() * flySpeed * TimeManager::GetDeltaTime();
			GetTransform()->SetPosition(newPos);
		}
		if (Input::GetKey(GLFW_KEY_S))
		{
			Vector3 newPos = GetTransform()->GetPosition() - GetTransform()->GetForward() * flySpeed * TimeManager::GetDeltaTime();
			GetTransform()->SetPosition(newPos);
		}
		if (Input::GetKey(GLFW_KEY_D))
		{
			Vector3 newPos = GetTransform()->GetPosition() + GetTransform()->GetRight() * flySpeed * TimeManager::GetDeltaTime();
			GetTransform()->SetPosition(newPos);
		}
		if (Input::GetKey(GLFW_KEY_A))
		{
			Vector3 newPos = GetTransform()->GetPosition() - GetTransform()->GetRight() * flySpeed * TimeManager::GetDeltaTime();
			GetTransform()->SetPosition(newPos);
		}
		if (Input::GetKey(GLFW_KEY_E))
		{
			Vector3 newPos = GetTransform()->GetPosition() + GetTransform()->GetUp() * flySpeed * TimeManager::GetDeltaTime();
			GetTransform()->SetPosition(newPos);
		}
		if (Input::GetKey(GLFW_KEY_Q))
		{
			Vector3 newPos = GetTransform()->GetPosition() - GetTransform()->GetUp() * flySpeed * TimeManager::GetDeltaTime();
			GetTransform()->SetPosition(newPos);
		}
	}
	else glfwSetInputMode(Application::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
