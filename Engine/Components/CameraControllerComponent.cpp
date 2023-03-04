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
		XMFLOAT3 currentPos = GetTransform()->GetPosition();
		XMFLOAT3 forward = GetTransform()->GetForward();

		XMVECTOR vec = DirectX::XMLoadFloat3(&currentPos) + DirectX::XMLoadFloat3(&forward) * scrollDelta * scrollSpeed;

		XMFLOAT3 newPos;
		DirectX::XMStoreFloat3(&newPos, vec);
		GetTransform()->SetPosition(newPos);
	}

	XMFLOAT2 posDelta = Input::GetMousePosDelta();

	XMVECTOR posDeltaVec = XMVector2ClampLength(DirectX::XMLoadFloat2(&posDelta), 0.f, 20.f);
	DirectX::XMStoreFloat2(&posDelta, posDeltaVec);

	if (Input::GetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE))
	{
		XMFLOAT3 currentPos = GetTransform()->GetPosition();
		XMFLOAT3 right = GetTransform()->GetRight();
		XMFLOAT3 up = GetTransform()->GetUp();

		XMVECTOR vec = DirectX::XMLoadFloat3(&currentPos) + DirectX::XMLoadFloat3(&right) * -posDelta.x * panSpeed * TimeManager::GetDeltaTime()
			+ DirectX::XMLoadFloat3(&up) * posDelta.y * panSpeed * TimeManager::GetDeltaTime();

		XMFLOAT3 newPos;
		DirectX::XMStoreFloat3(&newPos, vec);
		GetTransform()->SetPosition(newPos);
	}

	if (Input::GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
	{
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

		XMFLOAT3 right = GetTransform()->GetRight();
		XMVECTOR pitchVec = DirectX::XMQuaternionMultiply(yawVec, XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&right), pitch));

		XMFLOAT4 finalRot;
		DirectX::XMStoreFloat4(&finalRot, pitchVec);

		GetTransform()->SetRotation(finalRot);

		if (Input::GetKey(GLFW_KEY_W))
		{
			XMFLOAT3 currentPos = GetTransform()->GetPosition();
			XMFLOAT3 dir = GetTransform()->GetForward();

			XMVECTOR vec = DirectX::XMLoadFloat3(&currentPos) + DirectX::XMLoadFloat3(&dir) * flySpeed * TimeManager::GetDeltaTime();

			XMFLOAT3 newPos;
			DirectX::XMStoreFloat3(&newPos, vec);
			GetTransform()->SetPosition(newPos);
		}
		if (Input::GetKey(GLFW_KEY_S))
		{
			XMFLOAT3 currentPos = GetTransform()->GetPosition();
			XMFLOAT3 dir = GetTransform()->GetForward();

			XMVECTOR vec = DirectX::XMLoadFloat3(&currentPos) - DirectX::XMLoadFloat3(&dir) * flySpeed * TimeManager::GetDeltaTime();

			XMFLOAT3 newPos;
			DirectX::XMStoreFloat3(&newPos, vec);
			GetTransform()->SetPosition(newPos);
		}
		if (Input::GetKey(GLFW_KEY_D))
		{
			XMFLOAT3 currentPos = GetTransform()->GetPosition();
			XMFLOAT3 dir = GetTransform()->GetRight();

			XMVECTOR vec = DirectX::XMLoadFloat3(&currentPos) + DirectX::XMLoadFloat3(&dir) * flySpeed * TimeManager::GetDeltaTime();

			XMFLOAT3 newPos;
			DirectX::XMStoreFloat3(&newPos, vec);
			GetTransform()->SetPosition(newPos);
		}
		if (Input::GetKey(GLFW_KEY_A))
		{
			XMFLOAT3 currentPos = GetTransform()->GetPosition();
			XMFLOAT3 dir = GetTransform()->GetRight();

			XMVECTOR vec = DirectX::XMLoadFloat3(&currentPos) - DirectX::XMLoadFloat3(&dir) * flySpeed * TimeManager::GetDeltaTime();

			XMFLOAT3 newPos;
			DirectX::XMStoreFloat3(&newPos, vec);
			GetTransform()->SetPosition(newPos);
		}
		if (Input::GetKey(GLFW_KEY_E))
		{
			XMFLOAT3 currentPos = GetTransform()->GetPosition();
			XMFLOAT3 dir = GetTransform()->GetUp();

			XMVECTOR vec = DirectX::XMLoadFloat3(&currentPos) + DirectX::XMLoadFloat3(&dir) * flySpeed * TimeManager::GetDeltaTime();

			XMFLOAT3 newPos;
			DirectX::XMStoreFloat3(&newPos, vec);
			GetTransform()->SetPosition(newPos);
		}
		if (Input::GetKey(GLFW_KEY_Q))
		{
			XMFLOAT3 currentPos = GetTransform()->GetPosition();
			XMFLOAT3 dir = GetTransform()->GetUp();

			XMVECTOR vec = DirectX::XMLoadFloat3(&currentPos) - DirectX::XMLoadFloat3(&dir) * flySpeed * TimeManager::GetDeltaTime();

			XMFLOAT3 newPos;
			DirectX::XMStoreFloat3(&newPos, vec);
			GetTransform()->SetPosition(newPos);
		}

	}
}
