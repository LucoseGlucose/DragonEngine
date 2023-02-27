#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class SceneObject;

class Transform
{
	SceneObject* owner;
	Transform* parent;

	XMFLOAT3 position;
	XMFLOAT4 rotation;
	XMFLOAT3 scale;
	XMFLOAT4X4 matrix;

	XMFLOAT3 localPosition;
	XMFLOAT4 localRotation;
	XMFLOAT3 localScale;
	XMFLOAT4X4 localMatrix;

	void CalculateMatrices();

public:
	Transform(SceneObject* owner);

	SceneObject* GetOwner();
	Transform* GetParent();
	void SetParent(Transform* newParent, bool keepWorldTransform = true);

	XMFLOAT3 GetPosition();
	XMFLOAT4 GetRotation();
	XMFLOAT3 GetScale();
	XMFLOAT4X4 GetMatrix();

	XMFLOAT3 GetEulerAngles();
	XMFLOAT3 GetForward();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(XMFLOAT3 pos);
	void SetRotation(XMFLOAT4 rot);
	void SetScale(XMFLOAT3 scl);
	void SetMatrix(XMFLOAT4X4 mat);

	void SetEulerAngles(XMFLOAT3 angles);
	void SetForward(XMFLOAT3 fwd);
	void SetUp(XMFLOAT3 up);
	void SetRight(XMFLOAT3 right);

	XMFLOAT3 GetLocalPosition();
	XMFLOAT4 GetLocalRotation();
	XMFLOAT3 GetLocalScale();
	XMFLOAT4X4 GetLocalMatrix();

	void SetLocalPosition(XMFLOAT3 pos);
	void SetLocalRotation(XMFLOAT4 rot);
	void SetLocalScale(XMFLOAT3 scl);
	void SetLocalMatrix(XMFLOAT4X4 mat);

	XMFLOAT3 GetLocalEulerAngles();
	void SetLocalEulerAngles(XMFLOAT3 angles);

	float GetDistance(Transform* other, bool estimation);
};