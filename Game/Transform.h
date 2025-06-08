// ============================================
// 
// ファイル名: Transform.h
// 概要: Transform.cppのヘッダーファイル
// 
// 製作者 : 清水駿希
// 
// ============================================
#pragma once

class Transform
{
public:
    // 設定
    // ローカル座標を設定
    void SetLocalPosition(const DirectX::SimpleMath::Vector3& localPosition) { m_localPosition = localPosition; }
    // ローカル座標を設定
    void SetLocalRotation(const DirectX::SimpleMath::Quaternion& localRotation) { m_localRotation = localRotation; }
    // ローカルスケールを設定
    void SetLocalScale(const DirectX::SimpleMath::Vector3& localScale) { m_localScale = localScale; }

    // 取得
    // ローカル座標を設定
    DirectX::SimpleMath::Vector3 GetLocalPosition() const { return m_localPosition; }
    // ローカル回転角を設定
    DirectX::SimpleMath::Quaternion GetLocalRotation() const { return m_localRotation; }
    // ローカルスケールを設定
    DirectX::SimpleMath::Vector3 GetLocalScale() const { return m_localScale; }

    // ワールド行列を取得する
    DirectX::SimpleMath::Matrix GetWorldMatrix() const { return m_worldMatrix; }
    // ワールド座標を取得
    DirectX::SimpleMath::Vector3 GetWorldPosition() const;
    // ワールド回転を取得
    DirectX::SimpleMath::Quaternion GetWorldRotation() const;
    // ワールドスケールを取得
    DirectX::SimpleMath::Vector3 GetWorldScale() const;

    // 親を設定
    void SetParent(Transform* parent) { m_parent = parent; }
    // 親を取得する
    Transform* GetParent() const { return m_parent; }
    // 子供を設定する
    void SetChild(Transform* child) { m_childs.push_back(child); }
    // 子供を取得する
    std::vector<Transform*> GetChilds() const { return m_childs; }
    
public:
    // コンストラクタ
    Transform();
    // デストラクタ
    ~Transform() = default;

public:
    // 初期化
    void Initialize(
        DirectX::SimpleMath::Vector3 position ,
        DirectX::SimpleMath::Quaternion rotation,
        DirectX::SimpleMath::Vector3 scale
        );
    // 更新処理
    void Update();

private:

	// 親のTransform
	Transform* m_parent;
	// 子のTransform
	std::vector<Transform*> m_childs;

	// ローカル座標
	DirectX::SimpleMath::Vector3 m_localPosition;
	// ローカル回転角
	DirectX::SimpleMath::Quaternion m_localRotation;
	// ローカルスケール
	DirectX::SimpleMath::Vector3 m_localScale;

	// ワールド行列
	DirectX::SimpleMath::Matrix m_worldMatrix;
};