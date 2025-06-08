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
	// アクティブ状態のエミッターの更新処理
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
	// エミッター
	std::unique_ptr<ParticleEmitter> emitter;

	for (int i = 0; i < MAX_POOL; i++)
	{
		// エミッタークラスのインスタンスを生成する
		emitter.reset(new ParticleEmitter() );

		// 初期化処理
		emitter->Initialize(id);

		// 描画管理者に追加
		CommonResources::GetInstance()->GetRenderer()->Attach(emitter.get());

		// インスタンスを設定
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

		// 停止のメッセージだった場合
		if (!messageData.dataBool)
		{
			m_pools[ParametersID::PARTICLE][messageData.dataInt]->Stop();
			break;
		}
			
		for (const auto& emitter : m_pools[ParametersID::PARTICLE])
		{
			// 起動していないエミッターを再生する
			if (!emitter->GetIsActive())
			{
				// 再生
				emitter->Play();

				// 再生したオブジェクトにプール番号を渡す
				ObjectMessenger::GetInstance()->Dispatch(messageData.dataInt, { Message::MessageID::EFFECT_NUMBER , num });

				continue;
			}

			num++;
		}

		break;
	case Message::MessageID::SMOKE:

		// 停止のメッセージだった場合
		if (!messageData.dataBool)
		{
			m_pools[ParametersID::EFFECT][messageData.dataInt]->Stop();
			break;
		}

		for (const auto& emitter : m_pools[ParametersID::EFFECT])
		{
			// 起動していないエミッターを再生する
			if (!emitter->GetIsActive())
			{
				// 再生
				emitter->Play();

				// 再生したオブジェクトにプール番号を渡す
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

		// 停止のメッセージだった場合
		if (!messageData.dataBool)
		{
			m_pools[ParametersID::BALLOON_EXPLOSION][messageData.dataInt]->Stop();
			break;
		}

		for (const auto& emitter : m_pools[ParametersID::BALLOON_EXPLOSION])
		{
			// 起動していないエミッターを再生する
			if (!emitter->GetIsActive())
			{
				// 再生
				emitter->Play();

				// 再生したオブジェクトにプール番号を渡す
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