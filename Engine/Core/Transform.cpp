#include "stdafx.h"
#include "Transform.h"

Transform::Transform(SceneObject* owner) : Object(owner->name + " - Transform"), position(), rotation(Quaternion::Identity),
	scale(Vector3::One), matrix(Matrix::Identity), localPosition(), localRotation(Quaternion::Identity), localScale(Vector3::One), localMatrix()
{
	this->owner = owner;
	this->parent = nullptr;
}

SceneObject* Transform::GetOwner()
{
	return owner;
}

Transform* Transform::GetParent()
{
	return parent;
}

void Transform::SetParent(Transform* newParent)
{
	parent = newParent;

	onParentChanged(newParent);
}

Vector3 Transform::GetPosition()
{
	return position;
}

Quaternion Transform::GetRotation()
{
	return rotation;
}

Vector3 Transform::GetScale()
{
	return scale;
}

Matrix Transform::GetMatrix()
{
	return matrix;
}

Vector3 Transform::GetEulerAngles()
{
	return rotation.ToEuler();
}

Vector3 Transform::GetForward()
{
	return Vector3::Transform(Vector3::UnitZ, rotation);
}

Vector3 Transform::GetUp()
{
	return Vector3::Transform(Vector3::UnitY, rotation);
}

Vector3 Transform::GetRight()
{
	return Vector3::Transform(Vector3::UnitX, rotation);
}

void Transform::CalculateMatrices()
{
	matrix = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
	localMatrix = Matrix::CreateScale(localScale) * Matrix::CreateFromQuaternion(localRotation) * Matrix::CreateTranslation(localPosition);

	onMatrixChanged(matrix);
}

void Transform::SetPosition(Vector3 pos)
{
	if (pos == position) return;
	position = pos;

	if (parent == nullptr) localPosition = position;
	else localPosition = Vector3::Transform(position, parent->GetMatrix().Invert());

	CalculateMatrices();
	onPositionChanged(pos);
}

void Transform::SetRotation(Quaternion rot)
{
	if (rot == rotation) return;
	rotation = rot;

	if (parent == nullptr) localRotation = rotation;
	else localRotation = parent->GetRotation() * rotation;

	CalculateMatrices();
	onRotationChanged(rot);
}

void Transform::SetScale(Vector3 scl)
{
	if (scl == scale) return;
	scale = scl;

	if (parent == nullptr) localScale = scale;
	else localScale = scale / parent->GetScale();

	CalculateMatrices();
	onScaleChanged(scl);
}

void Transform::SetMatrix(Matrix mat)
{
	Vector3 pos;
	Quaternion rot;
	Vector3 scl;

	mat.Decompose(scl, rot, pos);

	SetPosition(pos);
	SetRotation(rot);
	SetScale(scl);
}

void Transform::SetEulerAngles(Vector3 angles)
{
	Quaternion rot = Quaternion::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(angles.x),
		DirectX::XMConvertToRadians(angles.y), DirectX::XMConvertToRadians(angles.z));

	SetRotation(rot);
}

void Transform::SetForward(Vector3 fwd)
{
	Quaternion rot = Quaternion::CreateFromAxisAngle(fwd, 0.f);\
	SetRotation(rot);
}

void Transform::SetUp(Vector3 up)
{
	Quaternion rot = Quaternion::CreateFromAxisAngle(up.Cross(GetRight()), 0.f);
	SetRotation(rot);
}

void Transform::SetRight(Vector3 right)
{
	Quaternion rot = Quaternion::CreateFromAxisAngle(right.Cross(GetUp()), 0.f);
	SetRotation(rot);
}

Vector3 Transform::GetLocalPosition()
{
	return localPosition;
}

Quaternion Transform::GetLocalRotation()
{
	return localRotation;
}

Vector3 Transform::GetLocalScale()
{
	return localScale;
}

Matrix Transform::GetLocalMatrix()
{
	return localMatrix;
}

void Transform::SetLocalPosition(Vector3 pos)
{
	if (pos == localPosition) return;
	localPosition = pos;

	if (parent == nullptr) position = localPosition;
	else position = Vector3::Transform(localPosition, parent->GetMatrix());

	CalculateMatrices();
}

void Transform::SetLocalRotation(Quaternion rot)
{
	if (rot == localRotation) return;
	localRotation = rot;

	if (parent == nullptr) rotation = localRotation;
	else rotation = parent->GetRotation() * localRotation;

	CalculateMatrices();
}

void Transform::SetLocalScale(Vector3 scl)
{
	if (scl == localScale) return;
	localScale = scl;

	if (parent == nullptr) scale = localScale;
	else scale = localScale * parent->GetScale();

	CalculateMatrices();
}

void Transform::SetLocalMatrix(Matrix mat)
{
	Vector3 pos;
	Quaternion rot;
	Vector3 scl;

	mat.Decompose(scl, rot, pos);

	SetLocalPosition(pos);
	SetLocalRotation(rot);
	SetLocalScale(scl);
}

Vector3 Transform::GetLocalEulerAngles()
{
	return localRotation.ToEuler();
}

void Transform::SetLocalEulerAngles(Vector3 angles)
{
	Quaternion rot = Quaternion::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(angles.x),
		DirectX::XMConvertToRadians(angles.y), DirectX::XMConvertToRadians(angles.z));

	SetLocalRotation(rot);
}

float Transform::GetDistance(Transform* other)
{
	return (position - other->GetPosition()).Length();
}
