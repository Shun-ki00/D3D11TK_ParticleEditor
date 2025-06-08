#include "pch.h"
#include "Game/Particle/EffectController.h"
#include "Game/Particle/ParticleEmitter.h"
#include "Framework/CommonResources.h"
#include "Game/Message/ObjectMessenger.h"

const int EffectController::MAX_POOL = 10;

EffectController::EffectController(IObject* parent, bool isActive, IObject::ObjectID objectId)
	:
	m_parent(parent),
	m_isActive(isActive),
	m_objectId(objectId),
	m_pools{}
{

}

void EffectController::Initialize()
{

}


void EffectController::Update(const float& elapsedTime)
{
	// �A�N�e�B�u��Ԃ̃G�~�b�^�[�̍X�V����
	for (auto& [id, emitters] : m_pools)
	{
		for (auto& emitter : emitters)
		{
			if (emitter->GetIsActive())
				emitter->Update(elapsedTime);
		}
	}
}


void EffectController::AddEffect(ParametersID id)
{
	// �G�~�b�^�[
	std::unique_ptr<ParticleEmitter> emitter;

	for (int i = 0; i < MAX_POOL; i++)
	{
		// �G�~�b�^�[�N���X�̃C���X�^���X�𐶐�����
		emitter.reset(new ParticleEmitter() );

		// ����������
		emitter->Initialize(id);

		// �`��Ǘ��҂ɒǉ�
		CommonResources::GetInstance()->GetRenderer()->Attach(emitter.get());

		// �C���X�^���X��ݒ�
		m_pools[id].push_back(std::move(emitter));
	}
}

void EffectController::Finalize()
{

}


void EffectController::OnMessegeAccepted(Message::MessageData messageData)
{
	int num = 0;

	switch (messageData.messageId)
	{
	case Message::MessageID::EXPLOSION:

		// ��~�̃��b�Z�[�W�������ꍇ
		if (!messageData.dataBool)
		{
			m_pools[ParametersID::PARTICLE][messageData.dataInt]->Stop();
			break;
		}
			
		for (const auto& emitter : m_pools[ParametersID::PARTICLE])
		{
			// �N�����Ă��Ȃ��G�~�b�^�[���Đ�����
			if (!emitter->GetIsActive())
			{
				// �Đ�
				emitter->Play();

				// �Đ������I�u�W�F�N�g�Ƀv�[���ԍ���n��
				ObjectMessenger::GetInstance()->Dispatch(messageData.dataInt, { Message::MessageID::EFFECT_NUMBER , num });

				continue;
			}

			num++;
		}

		break;
	case Message::MessageID::SMOKE:

		// ��~�̃��b�Z�[�W�������ꍇ
		if (!messageData.dataBool)
		{
			m_pools[ParametersID::EFFECT][messageData.dataInt]->Stop();
			break;
		}

		for (const auto& emitter : m_pools[ParametersID::EFFECT])
		{
			// �N�����Ă��Ȃ��G�~�b�^�[���Đ�����
			if (!emitter->GetIsActive())
			{
				// �Đ�
				emitter->Play();

				// �Đ������I�u�W�F�N�g�Ƀv�[���ԍ���n��
				ObjectMessenger::GetInstance()->Dispatch(messageData.dataInt, { Message::MessageID::EFFECT_NUMBER , num });

				continue;
			}

			num++;
		}


		break;
	case Message::MessageID::ATTACK:


		break;
	case Message::MessageID::MOVEMENT:
		break;

	case Message::MessageID::BALLOON_EXPLOSION:

		// ��~�̃��b�Z�[�W�������ꍇ
		if (!messageData.dataBool)
		{
			m_pools[ParametersID::BALLOON_EXPLOSION][messageData.dataInt]->Stop();
			break;
		}

		for (const auto& emitter : m_pools[ParametersID::BALLOON_EXPLOSION])
		{
			// �N�����Ă��Ȃ��G�~�b�^�[���Đ�����
			if (!emitter->GetIsActive())
			{
				// �Đ�
				emitter->Play();

				// �Đ������I�u�W�F�N�g�Ƀv�[���ԍ���n��
				ObjectMessenger::GetInstance()->Dispatch(messageData.dataInt, { Message::MessageID::EFFECT_NUMBER , num });

				continue;
			}

			num++;
		}


		break;


	default:
		break;
	}
}


void EffectController::OnKeyPressed(KeyType type, const DirectX::Keyboard::Keys& key)
{
	UNREFERENCED_PARAMETER(type);
	UNREFERENCED_PARAMETER(key);
}