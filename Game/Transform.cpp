// ============================================
// 
// �t�@�C����: Transform.cpp
// �T�v: Transform�̊K�w�Ǘ��ƃA�j���[�V��������
// 
// ����� : �����x��
// 
// ============================================
#include "pch.h"
#include "Game/Transform.h"


/// <summary>
/// �R���X�g���N�^
/// </summary>
Transform::Transform()
	:
	m_parent(nullptr)
{

	// ���[�J�����W�n��ݒ肷��
	m_localPosition = DirectX::SimpleMath::Vector3::Zero;
	m_localRotation = DirectX::SimpleMath::Quaternion::Identity;
	m_localScale    = DirectX::SimpleMath::Vector3::One;
}

/// <summary>
/// ����������
/// </summary>
/// <param name="position">�������W</param>
/// <param name="rotation">������]</param>
/// <param name="scale">�����X�P�[��</param>
void Transform::Initialize(
	DirectX::SimpleMath::Vector3 position,
	DirectX::SimpleMath::Quaternion rotation,
	DirectX::SimpleMath::Vector3 scale
)
{
	// ���[�J�����W�n��ݒ肷��
	m_localPosition = position;
	m_localRotation = rotation;
	m_localScale    = scale;
}

/// <summary>
/// �X�V����
/// </summary>
void Transform::Update()
{
	// ���[�J���s����v�Z����
		m_worldMatrix =
		DirectX::SimpleMath::Matrix::CreateScale(m_localScale) *
		DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_localRotation) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_localPosition);

	if (m_parent != nullptr)
	{
		// �q�̍s��ɐe�̕ψʕ�����悹����
		m_worldMatrix *= m_parent->GetWorldMatrix() ;
	}
}

// ���[���h���W���擾
DirectX::SimpleMath::Vector3 Transform::GetWorldPosition() const
{
	// ���[���h�s��̑�4�� (41, 42, 43) �����s�ړ�����
	return DirectX::SimpleMath::Vector3(m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43);
}

// ���[���h��]���擾
DirectX::SimpleMath::Quaternion Transform::GetWorldRotation() const
{
	using namespace DirectX::SimpleMath;

	// �X�P�[�����������ĉ�]�s������o��
	Vector3 scale = GetWorldScale();

	Matrix rotationMatrix = m_worldMatrix;
	rotationMatrix._11 /= scale.x;
	rotationMatrix._12 /= scale.x;
	rotationMatrix._13 /= scale.x;

	rotationMatrix._21 /= scale.y;
	rotationMatrix._22 /= scale.y;
	rotationMatrix._23 /= scale.y;

	rotationMatrix._31 /= scale.z;
	rotationMatrix._32 /= scale.z;
	rotationMatrix._33 /= scale.z;

	// ��]�s�񂩂�N�H�[�^�j�I�����쐬
	return Quaternion::CreateFromRotationMatrix(rotationMatrix);
}

// ���[���h�X�P�[�����擾
DirectX::SimpleMath::Vector3 Transform::GetWorldScale() const
{
	using namespace DirectX::SimpleMath;

	// �e���̃X�P�[�����v�Z
	float scaleX = Vector3(m_worldMatrix._11, m_worldMatrix._12, m_worldMatrix._13).Length();
	float scaleY = Vector3(m_worldMatrix._21, m_worldMatrix._22, m_worldMatrix._23).Length();
	float scaleZ = Vector3(m_worldMatrix._31, m_worldMatrix._32, m_worldMatrix._33).Length();

	return Vector3(scaleX, scaleY, scaleZ);
}