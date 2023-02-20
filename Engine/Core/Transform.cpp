#include "stdafx.h"
#include "Transform.h"

Transform::Transform(SceneObject* owner) : position(), rotation(DirectX::g_XMIdentityR3), scale(DirectX::g_XMOne3),
	matrix(), localPosition(), localRotation(DirectX::g_XMIdentityR3), localScale(DirectX::g_XMOne3), localMatrix()
{
	this->owner = owner;
	this->parent = nullptr;

	CalculateMatrices();
}

SceneObject* Transform::GetOwner()
{
	return owner;
}

Transform* Transform::GetParent()
{
	return parent;
}

void Transform::SetParent(Transform* newParent, bool keepWorldTransform)
{
	parent = newParent;
}

XMFLOAT3 Transform::GetPosition()
{
	return position;
}

XMFLOAT4 Transform::GetRotation()
{
	return rotation;
}

XMFLOAT3 Transform::GetScale()
{
	return scale;
}

XMFLOAT4X4 Transform::GetMatrix()
{
	return matrix;
}

XMFLOAT3 Transform::GetEulerAngles()
{
	return Utils::QuatToEulerAngles(rotation);
}

XMFLOAT3 Transform::GetForward()
{
	XMVECTOR rotVec = DirectX::XMLoadFloat4(&rotation);
	XMVECTOR fwdVec = XMVector3Transform(DirectX::g_XMIdentityR2, XMMatrixRotationQuaternion(rotVec));

	XMFLOAT3 fwd;
	DirectX::XMStoreFloat3(&fwd, fwdVec);

	return fwd;
}

XMFLOAT3 Transform::GetUp()
{
	XMVECTOR rotVec = DirectX::XMLoadFloat4(&rotation);
	XMVECTOR upVec = XMVector3Transform(DirectX::g_XMIdentityR1, XMMatrixRotationQuaternion(rotVec));

	XMFLOAT3 up;
	DirectX::XMStoreFloat3(&up, upVec);

	return up;
}

XMFLOAT3 Transform::GetRight()
{
	XMVECTOR rotVec = DirectX::XMLoadFloat4(&rotation);
	XMVECTOR rightVec = XMVector3Transform(DirectX::g_XMIdentityR0, XMMatrixRotationQuaternion(rotVec));

	XMFLOAT3 right;
	DirectX::XMStoreFloat3(&right, rightVec);

	return right;
}

void Transform::CalculateMatrices()
{
	XMVECTOR posVec = DirectX::XMLoadFloat3(&position);
	XMVECTOR rotVec = DirectX::XMLoadFloat4(&rotation);
	XMVECTOR sclVec = DirectX::XMLoadFloat3(&scale);

	XMMATRIX mat = DirectX::XMMatrixScalingFromVector(sclVec) *
		DirectX::XMMatrixRotationQuaternion(rotVec) * DirectX::XMMatrixTranslationFromVector(posVec);
	DirectX::XMStoreFloat4x4(&matrix, mat);

	XMVECTOR lposVec = DirectX::XMLoadFloat3(&localPosition);
	XMVECTOR lrotVec = DirectX::XMLoadFloat4(&localRotation);
	XMVECTOR lsclVec = DirectX::XMLoadFloat3(&localScale);

	XMMATRIX lmat = DirectX::XMMatrixScalingFromVector(lsclVec) *
		DirectX::XMMatrixRotationQuaternion(lrotVec) * DirectX::XMMatrixTranslationFromVector(lposVec);
	DirectX::XMStoreFloat4x4(&localMatrix, lmat);
}

void Transform::SetPosition(XMFLOAT3 pos)
{
	uint32_t comparison;
	DirectX::XMVectorEqualR(&comparison, DirectX::XMLoadFloat3(&position), DirectX::XMLoadFloat3(&pos));

	if (DirectX::XMComparisonAllTrue(comparison)) return;
	position = pos;

	if (parent == nullptr) localPosition = position;
	else
	{
		XMVECTOR posVec = DirectX::XMLoadFloat3(&position);

		XMFLOAT4X4 parent4x4Mat = parent->GetMatrix();
		XMMATRIX parentMat = DirectX::XMLoadFloat4x4(&parent4x4Mat);

		XMVECTOR parentMatDet = DirectX::XMMatrixDeterminant(parentMat);
		XMMATRIX invPMat = DirectX::XMMatrixInverse(&parentMatDet, parentMat);

		XMVECTOR newLPos = DirectX::XMVector3Transform(posVec, invPMat);
		DirectX::XMStoreFloat3(&localPosition, newLPos);
	}

	CalculateMatrices();
}

void Transform::SetRotation(XMFLOAT4 rot)
{
	uint32_t comparison;
	DirectX::XMVectorEqualR(&comparison, DirectX::XMLoadFloat4(&rotation), DirectX::XMLoadFloat4(&rot));

	if (DirectX::XMComparisonAllTrue(comparison)) return;
	rotation = rot;

	if (parent == nullptr) localRotation = rotation;
	else
	{
		XMVECTOR rotVec = DirectX::XMLoadFloat4(&rotation);

		XMFLOAT4 parentRot = parent->GetRotation();
		XMVECTOR parentRotVec = DirectX::XMLoadFloat4(&parentRot);
		XMVECTOR invParentRotVec = DirectX::XMQuaternionInverse(parentRotVec);

		XMVECTOR newLRot = DirectX::XMQuaternionMultiply(invParentRotVec, rotVec);
		DirectX::XMStoreFloat4(&localRotation, newLRot);
	}

	CalculateMatrices();
}

void Transform::SetScale(XMFLOAT3 scl)
{
	uint32_t comparison;
	DirectX::XMVectorEqualR(&comparison, DirectX::XMLoadFloat3(&scale), DirectX::XMLoadFloat3(&scl));

	if (DirectX::XMComparisonAllTrue(comparison)) return;
	scale = scl;

	if (parent == nullptr) localScale = scale;
	else
	{
		XMVECTOR sclVec = DirectX::XMLoadFloat3(&scale);

		XMFLOAT3 parentScale = parent->GetScale();
		XMVECTOR parentSclVec = DirectX::XMLoadFloat3(&parentScale);

		XMVECTOR newLScl = sclVec / parentSclVec;
		DirectX::XMStoreFloat3(&localScale, newLScl);
	}

	CalculateMatrices();
}

void Transform::SetMatrix(XMFLOAT4X4 mat)
{
	XMVECTOR posVec;
	XMVECTOR rotVec;
	XMVECTOR sclVec;
	XMMATRIX matMat = DirectX::XMLoadFloat4x4(&mat);

	DirectX::XMMatrixDecompose(&sclVec, &rotVec, &posVec, matMat);

	XMFLOAT3 pos;
	XMFLOAT4 rot;
	XMFLOAT3 scl;

	DirectX::XMStoreFloat3(&pos, posVec);
	DirectX::XMStoreFloat4(&rot, rotVec);
	DirectX::XMStoreFloat3(&scl, sclVec);

	SetPosition(pos);
	SetRotation(rot);
	SetScale(scl);
}

void Transform::SetEulerAngles(XMFLOAT3 angles)
{
	XMVECTOR rotVec = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(angles.x),
		DirectX::XMConvertToRadians(angles.y), DirectX::XMConvertToRadians(angles.z));

	XMFLOAT4 rot;
	DirectX::XMStoreFloat4(&rot, rotVec);

	SetRotation(rot);
}

void Transform::SetForward(XMFLOAT3 fwd)
{
	XMVECTOR fwdVec = DirectX::XMLoadFloat3(&fwd);
	XMVECTOR rotVec = DirectX::XMQuaternionRotationAxis(fwdVec, 0.0f);

	XMFLOAT4 rot;
	DirectX::XMStoreFloat4(&rot, rotVec);

	SetRotation(rot);
}

void Transform::SetUp(XMFLOAT3 up)
{
	XMVECTOR upVec = DirectX::XMLoadFloat3(&up);

	XMFLOAT3 right = GetRight();
	XMVECTOR rightVec = DirectX::XMLoadFloat3(&right);

	XMVECTOR rotVec = DirectX::XMQuaternionRotationAxis(DirectX::XMVector3Cross(upVec, rightVec), 0.0f);

	XMFLOAT4 rot;
	DirectX::XMStoreFloat4(&rot, rotVec);

	SetRotation(rot);
}

void Transform::SetRight(XMFLOAT3 right)
{
	XMVECTOR rightVec = DirectX::XMLoadFloat3(&right);

	XMFLOAT3 up = GetUp();
	XMVECTOR upVec = DirectX::XMLoadFloat3(&right);

	XMVECTOR rotVec = DirectX::XMQuaternionRotationAxis(DirectX::XMVector3Cross(rightVec, upVec), 0.0f);

	XMFLOAT4 rot;
	DirectX::XMStoreFloat4(&rot, rotVec);

	SetRotation(rot);
}

XMFLOAT3 Transform::GetLocalPosition()
{
	return localPosition;
}

XMFLOAT4 Transform::GetLocalRotation()
{
	return localRotation;
}

XMFLOAT3 Transform::GetLocalScale()
{
	return localScale;
}

XMFLOAT4X4 Transform::GetLocalMatrix()
{
	return localMatrix;
}

void Transform::SetLocalPosition(XMFLOAT3 pos)
{
	uint32_t comparison;
	DirectX::XMVectorEqualR(&comparison, DirectX::XMLoadFloat3(&localPosition), DirectX::XMLoadFloat3(&pos));

	if (DirectX::XMComparisonAllTrue(comparison)) return;
	localPosition = pos;

	if (parent == nullptr) position = localPosition;
	else
	{
		XMVECTOR posVec = DirectX::XMLoadFloat3(&localPosition);

		XMFLOAT4X4 parent4x4Mat = parent->GetMatrix();
		XMMATRIX parentMat = DirectX::XMLoadFloat4x4(&parent4x4Mat);

		XMVECTOR newWPos = DirectX::XMVector3Transform(posVec, parentMat);
		DirectX::XMStoreFloat3(&position, newWPos);
	}

	CalculateMatrices();
}

void Transform::SetLocalRotation(XMFLOAT4 rot)
{
	uint32_t comparison;
	DirectX::XMVectorEqualR(&comparison, DirectX::XMLoadFloat4(&localRotation), DirectX::XMLoadFloat4(&rot));

	if (DirectX::XMComparisonAllTrue(comparison)) return;
	localRotation = rot;

	if (parent == nullptr) rotation = localRotation;
	else
	{
		XMVECTOR rotVec = DirectX::XMLoadFloat4(&localRotation);

		XMFLOAT4 parentRot = parent->GetRotation();
		XMVECTOR parentRotVec = DirectX::XMLoadFloat4(&parentRot);

		XMVECTOR newWRot = DirectX::XMQuaternionMultiply(parentRotVec, rotVec);
		DirectX::XMStoreFloat4(&rotation, newWRot);
	}

	CalculateMatrices();
}

void Transform::SetLocalScale(XMFLOAT3 scl)
{
	uint32_t comparison;
	DirectX::XMVectorEqualR(&comparison, DirectX::XMLoadFloat3(&localScale), DirectX::XMLoadFloat3(&scl));

	if (DirectX::XMComparisonAllTrue(comparison)) return;
	localScale = scl;

	if (parent == nullptr) scale = localScale;
	else
	{
		XMVECTOR sclVec = DirectX::XMLoadFloat3(&localScale);

		XMFLOAT3 parentScale = parent->GetScale();
		XMVECTOR parentSclVec = DirectX::XMLoadFloat3(&parentScale);

		XMVECTOR newWScl = sclVec * parentSclVec;
		DirectX::XMStoreFloat3(&scale, newWScl);
	}

	CalculateMatrices();
}

void Transform::SetLocalMatrix(XMFLOAT4X4 mat)
{
	XMVECTOR posVec;
	XMVECTOR rotVec;
	XMVECTOR sclVec;
	XMMATRIX matMat = DirectX::XMLoadFloat4x4(&mat);

	DirectX::XMMatrixDecompose(&sclVec, &rotVec, &posVec, matMat);

	XMFLOAT3 pos;
	XMFLOAT4 rot;
	XMFLOAT3 scl;

	DirectX::XMStoreFloat3(&pos, posVec);
	DirectX::XMStoreFloat4(&rot, rotVec);
	DirectX::XMStoreFloat3(&scl, sclVec);

	SetLocalPosition(pos);
	SetLocalRotation(rot);
	SetLocalScale(scl);
}

XMFLOAT3 Transform::GetLocalEulerAngles()
{
	return Utils::QuatToEulerAngles(localRotation);
}

void Transform::SetLocalEulerAngles(XMFLOAT3 angles)
{
	XMVECTOR rotVec = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(angles.x),
		DirectX::XMConvertToRadians(angles.y), DirectX::XMConvertToRadians(angles.z));

	XMFLOAT4 rot;
	DirectX::XMStoreFloat4(&rot, rotVec);

	SetLocalRotation(rot);
}
