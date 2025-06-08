// ============================================
// 
// �t�@�C����: Transform.h
// �T�v: Transform.cpp�̃w�b�_�[�t�@�C��
// 
// ����� : �����x��
// 
// ============================================
#pragma once

class Transform
{
public:
    // �ݒ�
    // ���[�J�����W��ݒ�
    void SetLocalPosition(const DirectX::SimpleMath::Vector3& localPosition) { m_localPosition = localPosition; }
    // ���[�J�����W��ݒ�
    void SetLocalRotation(const DirectX::SimpleMath::Quaternion& localRotation) { m_localRotation = localRotation; }
    // ���[�J���X�P�[����ݒ�
    void SetLocalScale(const DirectX::SimpleMath::Vector3& localScale) { m_localScale = localScale; }

    // �擾
    // ���[�J�����W��ݒ�
    DirectX::SimpleMath::Vector3 GetLocalPosition() const { return m_localPosition; }
    // ���[�J����]�p��ݒ�
    DirectX::SimpleMath::Quaternion GetLocalRotation() const { return m_localRotation; }
    // ���[�J���X�P�[����ݒ�
    DirectX::SimpleMath::Vector3 GetLocalScale() const { return m_localScale; }

    // ���[���h�s����擾����
    DirectX::SimpleMath::Matrix GetWorldMatrix() const { return m_worldMatrix; }
    // ���[���h���W���擾
    DirectX::SimpleMath::Vector3 GetWorldPosition() const;
    // ���[���h��]���擾
    DirectX::SimpleMath::Quaternion GetWorldRotation() const;
    // ���[���h�X�P�[�����擾
    DirectX::SimpleMath::Vector3 GetWorldScale() const;

    // �e��ݒ�
    void SetParent(Transform* parent) { m_parent = parent; }
    // �e���擾����
    Transform* GetParent() const { return m_parent; }
    // �q����ݒ肷��
    void SetChild(Transform* child) { m_childs.push_back(child); }
    // �q�����擾����
    std::vector<Transform*> GetChilds() const { return m_childs; }
    
public:
    // �R���X�g���N�^
    Transform();
    // �f�X�g���N�^
    ~Transform() = default;

public:
    // ������
    void Initialize(
        DirectX::SimpleMath::Vector3 position ,
        DirectX::SimpleMath::Quaternion rotation,
        DirectX::SimpleMath::Vector3 scale
        );
    // �X�V����
    void Update();

private:

	// �e��Transform
	Transform* m_parent;
	// �q��Transform
	std::vector<Transform*> m_childs;

	// ���[�J�����W
	DirectX::SimpleMath::Vector3 m_localPosition;
	// ���[�J����]�p
	DirectX::SimpleMath::Quaternion m_localRotation;
	// ���[�J���X�P�[��
	DirectX::SimpleMath::Vector3 m_localScale;

	// ���[���h�s��
	DirectX::SimpleMath::Matrix m_worldMatrix;
};