#pragma once
#include "Interface/IObject.h"
#include "Game/Parameters/ParameterKeys.h"

class ParticleEmitter;

class EffectController : public IObject
{
private:

	// 最大プール数
	static const int MAX_POOL;

public:

	// オブジェクトのアクティブ設定
	void SetIsActive(const bool& isActive) override { m_isActive = isActive; }
	// オブジェクトのアクティブ状態を取得
	bool GetIsActive() const override { return m_isActive; }

	// オブジェクト番号を取得する
	int GetObjectNumber() const  override { return 0; }

	// オブジェクトIDを取得する
	IObject::ObjectID GetObjectID() const override { return m_objectId; }
	// Transformを取得する
	Transform* GetTransform() const override { return nullptr; }

	// 親オブジェクトを取得する
	IObject* GetParent() const override { return m_parent; }

	// コンストラクタ
	EffectController(IObject* parent , bool isActive , IObject::ObjectID objectId);

	// デストラクタ
	~EffectController() = default;

	// エフェクトを追加
	void AddEffect(ParametersID id);

public:
	
	// 初期化する
	void Initialize() override;
	// 更新する
	void Update(const float& elapsedTime) override;
	// 後処理を行う
	void Finalize() override;

	// メッセージを受け取る
	void OnMessegeAccepted(Message::MessageData messageData) override;

	// キーが押下げられたら時に呼び出される
	void OnKeyPressed(KeyType type, const DirectX::Keyboard::Keys& key) override;

private:

	bool m_isActive;

	IObject* m_parent;

	// オブジェクトID
	IObject::ObjectID m_objectId;

	std::unordered_map<ParametersID, std::vector<std::unique_ptr<ParticleEmitter>>> m_pools;

};