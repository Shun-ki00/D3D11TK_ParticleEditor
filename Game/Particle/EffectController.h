#pragma once
#include "Interface/IObject.h"
#include "Game/Parameters/ParameterKeys.h"

class ParticleEmitter;

class EffectController : public IObject
{
private:

	// �ő�v�[����
	static const int MAX_POOL;

public:

	// �I�u�W�F�N�g�̃A�N�e�B�u�ݒ�
	void SetIsActive(const bool& isActive) override { m_isActive = isActive; }
	// �I�u�W�F�N�g�̃A�N�e�B�u��Ԃ��擾
	bool GetIsActive() const override { return m_isActive; }

	// �I�u�W�F�N�g�ԍ����擾����
	int GetObjectNumber() const  override { return 0; }

	// �I�u�W�F�N�gID���擾����
	IObject::ObjectID GetObjectID() const override { return m_objectId; }
	// Transform���擾����
	Transform* GetTransform() const override { return nullptr; }

	// �e�I�u�W�F�N�g���擾����
	IObject* GetParent() const override { return m_parent; }

	// �R���X�g���N�^
	EffectController(IObject* parent , bool isActive , IObject::ObjectID objectId);

	// �f�X�g���N�^
	~EffectController() = default;

	// �G�t�F�N�g��ǉ�
	void AddEffect(ParametersID id);

public:
	
	// ����������
	void Initialize() override;
	// �X�V����
	void Update(const float& elapsedTime) override;
	// �㏈�����s��
	void Finalize() override;

	// ���b�Z�[�W���󂯎��
	void OnMessegeAccepted(Message::MessageData messageData) override;

	// �L�[����������ꂽ�玞�ɌĂяo�����
	void OnKeyPressed(KeyType type, const DirectX::Keyboard::Keys& key) override;

private:

	bool m_isActive;

	IObject* m_parent;

	// �I�u�W�F�N�gID
	IObject::ObjectID m_objectId;

	std::unordered_map<ParametersID, std::vector<std::unique_ptr<ParticleEmitter>>> m_pools;

};