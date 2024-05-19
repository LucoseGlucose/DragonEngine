#pragma once

#include "SimpleMath.h"
#include "Event.h"

using namespace DirectX::SimpleMath;

class SceneObject;

class Transform : public Object
{
	SceneObject* owner;
	Transform* parent;

	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	Matrix matrix;

	Vector3 localPosition;
	Quaternion localRotation;
	Vector3 localScale;
	Matrix localMatrix;

	void CalculateMatrices();

public:
	Transform(SceneObject* owner);

	SceneObject* GetOwner();
	Transform* GetParent();
	void SetParent(Transform* newParent);

	Vector3 GetPosition();
	Quaternion GetRotation();
	Vector3 GetScale();
	Matrix GetMatrix();

	Vector3 GetEulerAngles();
	Vector3 GetForward();
	Vector3 GetUp();
	Vector3 GetRight();

	void SetPosition(Vector3 pos);
	void SetRotation(Quaternion rot);
	void SetScale(Vector3 scl);
	void SetMatrix(Matrix mat);

	void SetEulerAngles(Vector3 angles);
	void SetForward(Vector3 fwd);
	void SetUp(Vector3 up);
	void SetRight(Vector3 right);

	Vector3 GetLocalPosition();
	Quaternion GetLocalRotation();
	Vector3 GetLocalScale();
	Matrix GetLocalMatrix();

	void SetLocalPosition(Vector3 pos);
	void SetLocalRotation(Quaternion rot);
	void SetLocalScale(Vector3 scl);
	void SetLocalMatrix(Matrix mat);

	Vector3 GetLocalEulerAngles();
	void SetLocalEulerAngles(Vector3 angles);

	float GetDistance(Transform* other);

	Event<Vector3> onPositionChanged;
	Event<Quaternion> onRotationChanged;
	Event<Vector3> onScaleChanged;
	Event<Matrix> onMatrixChanged;
	Event<Transform*> onParentChanged;
};